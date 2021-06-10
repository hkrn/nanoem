/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/EditingModelTrait.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {

EditingModelTrait::Base::ModelScope::ModelScope(Model *model)
    : m_value(nanoemMutableModelCreateAsReference(model->data(), nullptr))
{
}

EditingModelTrait::Base::ModelScope::~ModelScope() NANOEM_DECL_NOEXCEPT
{
    nanoemMutableModelDestroy(m_value);
}

EditingModelTrait::Base::Base(Model *model)
    : m_model(model)
{
}

EditingModelTrait::Base::~Base() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Base::handleStatus(nanoem_status_t status)
{
    if (status != NANOEM_STATUS_SUCCESS) {
        Project *project = m_model->project();
        char message[Error::kMaxReasonLength];
        StringUtils::format(message, sizeof(message), "Cannot modify the model: %s",
            Error::convertStatusToMessage(status, project->translator()));
        Error error(message, status, Error::kDomainTypeNanoem);
        error.notify(project->eventPublisher());
    }
}

EditingModelTrait::Vertex::Vertex(Model *model)
    : Base(model)
{
}

EditingModelTrait::Vertex::~Vertex() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Vertex::normalizeAllVertices()
{
    typedef tinystl::vector<Vector3, TinySTLAllocator> Vector3List;
    typedef tinystl::unordered_map<nanoem_u32_t, Vector3List, TinySTLAllocator> FaceNormalList;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_rsize_t numObjects, numIndices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numObjects);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(opaque, &numIndices);
    FaceNormalList faceNormalList;
    for (nanoem_rsize_t i = 0; i < numIndices; i += 3) {
        const nanoem_u32_t index0 = indices[i + 0];
        const nanoem_u32_t index1 = indices[i + 1];
        const nanoem_u32_t index2 = indices[i + 2];
        const nanoem_model_vertex_t *vertexPtr0 = vertices[index0];
        const nanoem_model_vertex_t *vertexPtr1 = vertices[index1];
        const nanoem_model_vertex_t *vertexPtr2 = vertices[index2];
        const Vector3 v0(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr0)));
        const Vector3 v1(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr1)));
        const Vector3 v2(glm::make_vec3(nanoemModelVertexGetOrigin(vertexPtr2)));
        const Vector3 n(glm::normalize(glm::cross(v1 - v0, v2 - v1)));
        if (!glm::any(glm::isinf(n)) && !glm::any(glm::isnan(n))) {
            faceNormalList[index0].push_back(n);
            faceNormalList[index1].push_back(n);
            faceNormalList[index2].push_back(n);
        }
    }
    for (FaceNormalList::const_iterator it = faceNormalList.begin(), end = faceNormalList.end(); it != end; ++it) {
        const Vector3List &normals = it->second;
        if (!normals.empty()) {
            Vector3 normal(Constants::kZeroV3);
            for (Vector3List::const_iterator it2 = normals.begin(), end2 = normals.end(); it2 != end2; ++it2) {
                normal += *it2;
            }
            if (glm::length(normal) > 0) {
                nanoem_mutable_model_vertex_t *vertex =
                    nanoemMutableModelVertexCreateAsReference(vertices[it->first], &status);
                nanoemMutableModelVertexSetNormal(vertex, glm::value_ptr(Vector4(glm::normalize(normal), 0)));
                nanoemMutableModelVertexDestroy(vertex);
            }
        }
    }
}

model::Material::MutableList
EditingModelTrait::Material::sortedAscent(const model::Material::MutableSet &value)
{
    model::Material::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::Material::MutableList
EditingModelTrait::Material::sortedDescent(const model::Material::MutableSet &value)
{
    model::Material::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::Material::Material(Model *model)
    : Base(model)
{
}

EditingModelTrait::Material::~Material() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Material::addItem()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_material_t *material =
        nanoemMutableModelMaterialCreate(nanoemMutableModelGetOriginObject(scope.m_value), &status);
    nanoem_model_material_t *origin = nanoemMutableModelMaterialGetOriginObject(material);
    nanoemMutableModelInsertMaterialObject(scope.m_value, material, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Material%d", nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(origin)));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelMaterialSetName(material, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelMaterialSetName(material, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Material *m = model::Material::create(project->sharedFallbackImage());
    m->bind(origin);
    m->resetLanguage(origin, factory, project->castLanguage());
    nanoemMutableModelMaterialDestroy(material);
    handleStatus(status);
}

void
EditingModelTrait::Material::removeItem(const model::Material::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    MaterialVertexIndicesMap mvi;
    getMaterialVertexIndicesMap(mvi);
    for (model::Material::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_material_t *item = nanoemMutableModelMaterialCreateAsReference(*it, &status);
        nanoemMutableModelRemoveMaterialObject(scope.m_value, item, &status);
        MaterialVertexIndicesMap::const_iterator it2 = mvi.find(*it);
        if (it2 != mvi.end()) {
            mvi.erase(it2);
        }
        nanoemMutableModelMaterialDestroy(item);
    }
    resetVertexIndices(mvi);
    handleStatus(status);
}

void
EditingModelTrait::Material::moveItemTop(const model::Material::MutableSet &value)
{
    moveItemAt(value, 0);
}

void
EditingModelTrait::Material::moveItemUp(const model::Material::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Material::moveItemDown(const model::Material::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Material::moveItemBottom(const model::Material::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::Material::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::Material::MutableList, TinySTLAllocator> NamedMaterialList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_opaque, &numObjects);
    NamedMaterialList namedMaterials;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_material_t *materialPtr = materials[i];
        if (const model::Material *material = model::Material::cast(materialPtr)) {
            namedMaterials[material->canonicalName()].push_back(materialPtr);
        }
    }
    for (NamedMaterialList::const_iterator it = namedMaterials.begin(), end = namedMaterials.end(); it != end; ++it) {
        const model::Material::MutableList &m = it->second;
        if (m.size() > 1) {
            model::Material::MutableSet set;
            for (model::Material::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2; ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

void
EditingModelTrait::Material::createMaterial(const OBJLoader *loader)
{
    BX_UNUSED_1(loader);
}

int
EditingModelTrait::Material::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::Material::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::Material::compareCommon(const void *left, const void *right)
{
    const nanoem_model_material_t *lvalue = *static_cast<nanoem_model_material_t *const *>(left);
    const nanoem_model_material_t *rvalue = *static_cast<nanoem_model_material_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(rvalue));
}

void
EditingModelTrait::Material::moveItemAt(const model::Material::MutableSet &value, int offset)
{
    model::Material::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Material::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_material_t *item = nanoemMutableModelMaterialCreateAsReference(*it, &status);
        nanoemMutableModelRemoveMaterialObject(scope.m_value, item, &status);
        nanoemMutableModelInsertMaterialObject(scope.m_value, item, offset, &status);
        nanoemMutableModelMaterialDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Material::getMaterialVertexIndicesMap(MaterialVertexIndicesMap &mvi)
{
    nanoem_rsize_t numMaterials, numVertexIndices, offset = 0;
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(m_model->data(), &numVertexIndices);
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_model->data(), &numMaterials);
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        nanoem_model_material_t *material = materials[i];
        nanoem_rsize_t numIndices = nanoemModelMaterialGetNumVertexIndices(material);
        mvi.insert(tinystl::make_pair(material, tinystl::make_pair(indices + offset, numIndices)));
        offset += numIndices;
    }
}

void
EditingModelTrait::Material::resetVertexIndices(const MaterialVertexIndicesMap &mvi)
{
    nanoem_rsize_t numMaterials;
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(m_model->data(), &numMaterials);
    VertexIndexList indices;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        nanoem_model_material_t *material = materials[i];
        MaterialVertexIndicesMap::const_iterator it2 = mvi.find(material);
        if (it2 != mvi.end()) {
            for (nanoem_rsize_t j = 0, numIndices = it2->second.second; j < numIndices; j++) {
                indices.push_back(it2->second.first[j]);
            }
        }
    }
    ModelScope scope(m_model);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoemMutableModelSetVertexIndices(scope.m_value, indices.data(), indices.size(), &status);
    handleStatus(status);
}

model::Bone::MutableList
EditingModelTrait::Bone::sortedAscent(const model::Bone::MutableSet &value)
{
    model::Bone::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::Bone::MutableList
EditingModelTrait::Bone::sortedDescent(const model::Bone::MutableSet &value)
{
    model::Bone::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::Bone::Bone(Model *model)
    : Base(model)
{
}

EditingModelTrait::Bone::~Bone() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Bone::addItem()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_bone_t *bone =
        nanoemMutableModelBoneCreate(nanoemMutableModelGetOriginObject(scope.m_value), &status);
    nanoem_model_bone_t *origin = nanoemMutableModelBoneGetOriginObject(bone);
    nanoemMutableModelBoneSetRotateable(bone, 1);
    nanoemMutableModelBoneSetVisible(bone, 1);
    nanoemMutableModelBoneSetUserHandleable(bone, 1);
    nanoemMutableModelBoneSetDestinationOrigin(bone, glm::value_ptr(Vector4()));
    nanoemMutableModelInsertBoneObject(scope.m_value, bone, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Bone%d", nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(origin)));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelBoneSetName(bone, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelBoneSetName(bone, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Bone *m = model::Bone::create();
    m->bind(origin);
    m->resetLanguage(origin, factory, project->castLanguage());
    nanoemMutableModelBoneDestroy(bone);
    handleStatus(status);
}

void
EditingModelTrait::Bone::removeItem(const model::Bone::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Bone::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_bone_t *item = nanoemMutableModelBoneCreateAsReference(*it, &status);
        nanoemMutableModelRemoveBoneObject(scope.m_value, item, &status);
        nanoemMutableModelBoneDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Bone::moveItemTop(const model::Bone::MutableSet &value)
{
    moveItemAt(value, 0);
}

void
EditingModelTrait::Bone::moveItemUp(const model::Bone::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Bone::moveItemDown(const model::Bone::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Bone::moveItemBottom(const model::Bone::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::Bone::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::Bone::MutableList, TinySTLAllocator> NamedBoneList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_opaque, &numObjects);
    NamedBoneList namedBones;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_bone_t *bonePtr = bones[i];
        if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
            namedBones[bone->canonicalName()].push_back(bonePtr);
        }
    }
    for (NamedBoneList::const_iterator it = namedBones.begin(), end = namedBones.end(); it != end; ++it) {
        const model::Bone::MutableList &m = it->second;
        if (m.size() > 1) {
            model::Bone::MutableSet set;
            for (model::Bone::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2; ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

int
EditingModelTrait::Bone::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::Bone::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::Bone::compareCommon(const void *left, const void *right)
{
    const nanoem_model_bone_t *lvalue = *static_cast<nanoem_model_bone_t *const *>(left);
    const nanoem_model_bone_t *rvalue = *static_cast<nanoem_model_bone_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(rvalue));
}

void
EditingModelTrait::Bone::createRootParentBone()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_bone_t *rootBone = nanoemMutableModelBoneCreate(m_model->data(), &status);
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    nanoem_rsize_t numObjects;
    StringUtils::UnicodeStringScope s(factory);
    if (StringUtils::tryGetString(factory, reinterpret_cast<const char *>(model::Bone::kNameRootParentInJapanese), s)) {
        nanoemMutableModelBoneSetName(rootBone, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, "root", s)) {
        nanoemMutableModelBoneSetName(rootBone, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelBoneSetParentBoneObject(rootBone, nullptr);
    {
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numObjects);
        if (numObjects > 0) {
            nanoemMutableModelBoneSetTargetBoneObject(rootBone, bones[0]);
        }
    }
    nanoemMutableModelBoneSetMovable(rootBone, 1);
    nanoemMutableModelBoneSetRotateable(rootBone, 1);
    nanoemMutableModelBoneSetUserHandleable(rootBone, 1);
    nanoemMutableModelBoneSetVisible(rootBone, 1);
    String name;
    nanoem_rsize_t numItems;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_model->data(), &numObjects);
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_label_t *labelPtr = labels[i];
        model::Label *label = model::Label::cast(labelPtr);
        if (StringUtils::equals(label->nameConstString(), "Root") && nanoemModelLabelIsSpecial(labelPtr)) {
            nanoem_mutable_model_label_t *mutableLabel = nanoemMutableModelLabelCreateAsReference(labelPtr, &status);
            nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numItems);
            for (nanoem_rsize_t j = 0; j < numItems; j++) {
                nanoem_mutable_model_label_item_t *mutableLabelItem =
                    nanoemMutableModelLabelItemCreateAsReference(items[i], &status);
                nanoemMutableModelLabelRemoveItemObject(mutableLabel, mutableLabelItem, &status);
                nanoemMutableModelLabelItemDestroy(mutableLabelItem);
            }
            nanoem_mutable_model_label_item_t *newItem = nanoemMutableModelLabelItemCreateFromBoneObject(
                mutableLabel, nanoemMutableModelBoneGetOriginObject(rootBone), &status);
            nanoemMutableModelLabelInsertItemObject(mutableLabel, newItem, 0, &status);
            nanoemMutableModelLabelItemDestroy(newItem);
            nanoemMutableModelLabelDestroy(mutableLabel);
            break;
        }
    }
    ModelScope scope(m_model);
    nanoemMutableModelInsertBoneObject(scope.m_value, rootBone, 0, &status);
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_model->data(), &numObjects);
    for (nanoem_rsize_t i = 1; i < numObjects; i++) {
        nanoem_model_bone_t *bone = bones[i];
        if (!nanoemModelBoneGetParentBoneObject(bone)) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bone, &status);
            nanoemMutableModelBoneSetParentBoneObject(mutableBone, nanoemMutableModelBoneGetOriginObject(rootBone));
            nanoemMutableModelBoneDestroy(mutableBone);
        }
    }
    bind(rootBone);
    nanoemMutableModelBoneDestroy(rootBone);
    handleStatus(status);
}

void
EditingModelTrait::Bone::createStagingProxyBone(nanoem_model_bone_t *baseBone)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_mutable_model_bone_t *newParentBone = nanoemMutableModelBoneCreate(opaque, &status);
    int newStageIndex = nanoemModelBoneGetStageIndex(baseBone) + 1;
    bind(newParentBone);
    {
        ModelScope scope(m_model);
        int offset = nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(baseBone)) + 1;
        nanoemMutableModelInsertBoneObject(scope.m_value, newParentBone, offset, &status);
        nanoemMutableModelBoneCopy(newParentBone, baseBone, &status);
        setStagingProxyBoneName(newParentBone, baseBone, newStageIndex, NANOEM_LANGUAGE_TYPE_JAPANESE);
        setStagingProxyBoneName(newParentBone, baseBone, newStageIndex, NANOEM_LANGUAGE_TYPE_ENGLISH);
        // nanoemMutableModelBoneSetStageIndex(newParentBone, newStageIndex);
    }
    nanoem_model_bone_t *newBoneOrigin = nanoemMutableModelBoneGetOriginObject(newParentBone);
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(opaque, &numBones);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        nanoem_model_bone_t *bone = bones[i];
        if (nanoemModelBoneGetParentBoneObject(bone) == baseBone) {
            nanoem_mutable_model_bone_t *mutableBone = nanoemMutableModelBoneCreateAsReference(bone, &status);
            // nanoemMutableModelBoneSetStageIndex(mutableBone, newStageIndex);
            nanoemMutableModelBoneSetParentBoneObject(mutableBone, newBoneOrigin);
            nanoemMutableModelBoneDestroy(mutableBone);
        }
    }
    nanoemMutableModelBoneSetParentBoneObject(newParentBone, baseBone);
    {
        nanoem_model_bone_t *targetBone = bones[nanoemModelObjectGetIndex(
            nanoemModelBoneGetModelObject(nanoemModelBoneGetTargetBoneObject(baseBone)))];
        nanoem_mutable_model_bone_t *mutableTargetBone = nanoemMutableModelBoneCreateAsReference(targetBone, &status);
        nanoemMutableModelBoneSetStageIndex(mutableTargetBone, newStageIndex);
        nanoemMutableModelBoneDestroy(mutableTargetBone);
        nanoem_mutable_model_bone_t *mutableBaseBone = nanoemMutableModelBoneCreateAsReference(baseBone, &status);
        nanoemMutableModelBoneSetTargetBoneObject(mutableBaseBone, newBoneOrigin);
        nanoemMutableModelBoneDestroy(mutableBaseBone);
    }
    nanoemMutableModelBoneDestroy(newParentBone);
    handleStatus(status);
}

void
EditingModelTrait::Bone::createLabel(const model::Bone::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_label_t *label = nanoemMutableModelLabelCreate(m_model->data(), &status);
    const model::Bone::MutableList &bones = sortedAscent(value);
    for (model::Bone::MutableList::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
        nanoem_mutable_model_label_item_t *item = nanoemMutableModelLabelItemCreateFromBoneObject(label, *it, &status);
        nanoemMutableModelLabelInsertItemObject(label, item, -1, &status);
        nanoemMutableModelLabelItemDestroy(item);
    }
    nanoemMutableModelInsertLabelObject(scope.m_value, label, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Label%d",
        nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(nanoemMutableModelLabelGetOriginObject(label))));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    model::Label *l = model::Label::create();
    l->bind(nanoemMutableModelLabelGetOriginObject(label));
    l->resetLanguage(nanoemMutableModelLabelGetOriginObject(label), factory, project->castLanguage());
    nanoemMutableModelLabelDestroy(label);
    handleStatus(status);
}

void
EditingModelTrait::Bone::createConstraintChain(const model::Bone::MutableSet &value)
{
    if (value.size() >= 2) {
        const model::Bone::MutableList &bones = sortedDescent(value);
        nanoem_status_t status = NANOEM_STATUS_SUCCESS;
        nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
        ModelScope scope(m_model);
        nanoem_mutable_model_bone_t *bone = nanoemMutableModelBoneCreate(m_model->data(), &status);
        StringUtils::UnicodeStringScope s(factory);
        String name;
        StringUtils::format(name, "Constraint");
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelBoneSetName(bone, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        }
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelBoneSetName(bone, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
        nanoemMutableModelInsertBoneObject(scope.m_value, bone, -1, &status);
        bind(bone);
        nanoem_mutable_model_constraint_t *constraint = nanoemMutableModelConstraintCreate(m_model->data(), &status);
        nanoemMutableModelConstraintSetAngleLimit(constraint, 0);
        nanoemMutableModelConstraintSetEffectorBoneObject(constraint, bones.front());
        nanoemMutableModelConstraintSetNumIterations(constraint, 16);
        nanoemMutableModelConstraintSetTargetBoneObject(constraint, nanoemMutableModelBoneGetOriginObject(bone));
        for (model::Bone::MutableList::const_iterator it = bones.begin() + 1, end = bones.end(); it != end; ++it) {
            nanoem_mutable_model_constraint_joint_t *joint =
                nanoemMutableModelConstraintJointCreate(constraint, &status);
            nanoemMutableModelConstraintJointSetBoneObject(joint, *it);
            nanoemMutableModelConstraintJointSetLowerLimit(joint, glm::value_ptr(Vector4(0)));
            nanoemMutableModelConstraintJointSetUpperLimit(joint, glm::value_ptr(Vector4(0)));
            nanoemMutableModelConstraintInsertJointObject(constraint, joint, -1, &status);
            nanoemMutableModelConstraintJointDestroy(joint);
        }
        nanoemMutableModelBoneSetConstraintEnabled(bone, 1);
        nanoemMutableModelBoneSetConstraintObject(bone, constraint);
        nanoemMutableModelBoneDestroy(bone);
        bind(constraint);
        nanoemMutableModelConstraintDestroy(constraint);
        handleStatus(status);
    }
}

void
EditingModelTrait::Bone::moveItemAt(const model::Bone::MutableSet &value, int offset)
{
    model::Bone::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Bone::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_bone_t *item = nanoemMutableModelBoneCreateAsReference(*it, &status);
        nanoemMutableModelRemoveBoneObject(scope.m_value, item, &status);
        nanoemMutableModelInsertBoneObject(scope.m_value, item, offset, &status);
        nanoemMutableModelBoneDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Bone::setStagingProxyBoneName(nanoem_mutable_model_bone_t *newBone, const nanoem_model_bone_t *bone,
    int newStageIndex, nanoem_language_type_t language)
{
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t length;
    nanoem_u8_t *bytes =
        nanoemUnicodeStringFactoryGetByteArray(factory, nanoemModelBoneGetName(bone, language), &length, &status);
    char buffer[20];
    StringUtils::format(buffer, sizeof(buffer), "%s+%d", bytes, newStageIndex);
    nanoem_unicode_string_t *newName = nanoemUnicodeStringFactoryCreateString(
        factory, reinterpret_cast<const nanoem_u8_t *>(buffer), StringUtils::length(buffer), &status);
    nanoemMutableModelBoneSetName(newBone, newName, language, &status);
    nanoemUnicodeStringFactoryDestroyString(factory, newName);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    handleStatus(status);
}

void
EditingModelTrait::Bone::bind(nanoem_mutable_model_bone_t *bonePtr)
{
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    model::Bone *bone = model::Bone::create();
    nanoem_model_bone_t *originPtr = nanoemMutableModelBoneGetOriginObject(bonePtr);
    bone->bind(originPtr);
    bone->resetLanguage(originPtr, factory, project->castLanguage());
}

void
EditingModelTrait::Bone::bind(nanoem_mutable_model_constraint_t *constraintPtr)
{
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    model::Constraint *constraint = model::Constraint::create();
    nanoem_model_constraint_t *originPtr = nanoemMutableModelConstraintGetOriginObject(constraintPtr);
    constraint->bind(originPtr);
    constraint->resetLanguage(originPtr, factory, project->castLanguage());
    constraint->initialize(originPtr);
}

model::Morph::MutableList
EditingModelTrait::Morph::sortedAscent(const model::Morph::MutableSet &value)
{
    model::Morph::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::Morph::MutableList
EditingModelTrait::Morph::sortedDescent(const model::Morph::MutableSet &value)
{
    model::Morph::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::Morph::Morph(Model *model)
    : Base(model)
{
}

EditingModelTrait::Morph::~Morph() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Morph::addItem()
{
}

void
EditingModelTrait::Morph::removeItem(const model::Morph::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Morph::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_morph_t *item = nanoemMutableModelMorphCreateAsReference(*it, &status);
        nanoemMutableModelRemoveMorphObject(scope.m_value, item, &status);
        nanoemMutableModelMorphDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Morph::moveItemTop(const model::Morph::MutableSet &value)
{
    moveItemAt(value, 0);
}

void
EditingModelTrait::Morph::moveItemUp(const model::Morph::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Morph::moveItemDown(const model::Morph::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Morph::moveItemBottom(const model::Morph::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::Morph::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::Morph::MutableList, TinySTLAllocator> NamedMorphList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(m_opaque, &numObjects);
    NamedMorphList namedMorphs;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_morph_t *morphPtr = morphs[i];
        if (const model::Morph *morph = model::Morph::cast(morphPtr)) {
            namedMorphs[morph->canonicalName()].push_back(morphPtr);
        }
    }
    for (NamedMorphList::const_iterator it = namedMorphs.begin(), end = namedMorphs.end(); it != end; ++it) {
        const model::Morph::MutableList &m = it->second;
        if (m.size() > 1) {
            model::Morph::MutableSet set;
            for (model::Morph::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2; ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

void
EditingModelTrait::Morph::createVertexMorph(const OBJLoader *loader)
{
    BX_UNUSED_1(loader);
}

void
EditingModelTrait::Morph::createGroupMorphFromCurrentPose()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_rsize_t numMorphs;
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(opaque, &numMorphs);
    nanoem_mutable_model_morph_t *parentMorph = nanoemMutableModelMorphCreate(opaque, &status);
    nanoem_rsize_t numItems = 0;
    nanoemMutableModelMorphSetCategory(parentMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    nanoemMutableModelMorphSetType(parentMorph, NANOEM_MODEL_MORPH_TYPE_GROUP);
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        const model::Morph *morph = model::Morph::cast(morphs[i]);
        nanoem_f32_t weight = morph->weight();
        if (weight != 0) {
            nanoem_mutable_model_morph_group_t *item = nanoemMutableModelMorphGroupCreate(parentMorph, &status);
            nanoemMutableModelMorphGroupSetMorphObject(item, morphs[i]);
            nanoemMutableModelMorphGroupSetWeight(item, weight);
            nanoemMutableModelMorphInsertGroupMorphObject(parentMorph, item, -1, &status);
            nanoemMutableModelMorphGroupDestroy(item);
            numItems++;
        }
    }
    if (numItems > 0) {
        nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
        String name;
        nanoem_rsize_t numObjects;
        nanoemModelGetAllMorphObjects(opaque, &numObjects);
        StringUtils::format(name, "Morph%jd", numObjects + 1);
        StringUtils::UnicodeStringScope s(factory);
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        }
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
        ModelScope scope(m_model);
        nanoemMutableModelInsertMorphObject(scope.m_value, parentMorph, -1, &status);
        bind(parentMorph);
    }
    nanoemMutableModelMorphDestroy(parentMorph);
    handleStatus(status);
}

void
EditingModelTrait::Morph::createBoneMorphFromCurrentPose()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_rsize_t numBones;
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(opaque, &numBones);
    nanoem_mutable_model_morph_t *parentMorph = nanoemMutableModelMorphCreate(opaque, &status);
    nanoem_rsize_t numItems = 0;
    nanoemMutableModelMorphSetCategory(parentMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    nanoemMutableModelMorphSetType(parentMorph, NANOEM_MODEL_MORPH_TYPE_BONE);
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const model::Bone *bone = model::Bone::cast(bonePtr);
        const Vector3 p(bone->localTranslation());
        const Quaternion &q = bone->localOrientation();
        if (glm::distance(p, Constants::kZeroV3) > 0 || glm::any(glm::notEqual(q, Constants::kZeroQ))) {
            nanoem_mutable_model_morph_bone_t *item = nanoemMutableModelMorphBoneCreate(parentMorph, &status);
            nanoemMutableModelMorphBoneSetBoneObject(item, bonePtr);
            nanoemMutableModelMorphBoneSetTranslation(item, glm::value_ptr(Vector4(p, 1)));
            nanoemMutableModelMorphBoneSetOrientation(item, glm::value_ptr(q));
            nanoemMutableModelMorphInsertBoneMorphObject(parentMorph, item, -1, &status);
            nanoemMutableModelMorphBoneDestroy(item);
            numItems++;
        }
    }
    if (numItems > 0) {
        nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
        String name;
        nanoem_rsize_t numObjects;
        nanoemModelGetAllMorphObjects(opaque, &numObjects);
        StringUtils::format(name, "Morph%jd", numObjects + 1);
        StringUtils::UnicodeStringScope s(factory);
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        }
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
        ModelScope scope(m_model);
        nanoemMutableModelInsertMorphObject(scope.m_value, parentMorph, -1, &status);
        bind(parentMorph);
    }
    nanoemMutableModelMorphDestroy(parentMorph);
    handleStatus(status);
}

void
EditingModelTrait::Morph::createVertexMorphFromCurrentPose()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_rsize_t numVertexs;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(opaque, &numVertexs);
    nanoem_mutable_model_morph_t *parentMorph = nanoemMutableModelMorphCreate(opaque, &status);
    tinystl::unordered_set<const nanoem_model_vertex_t *, TinySTLAllocator> added;
    nanoemMutableModelMorphSetCategory(parentMorph, NANOEM_MODEL_MORPH_CATEGORY_OTHER);
    nanoemMutableModelMorphSetType(parentMorph, NANOEM_MODEL_MORPH_TYPE_VERTEX);
    for (nanoem_rsize_t i = 0; i < numVertexs; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        for (int j = 0; j < 4; j++) {
            if (added.find(vertexPtr) == added.end()) {
                if (const model::Bone *bone = model::Bone::cast(nanoemModelVertexGetBoneObject(vertexPtr, j))) {
                    const Vector3 p(bone->skinningTransform()[3]);
                    if (glm::distance(p, Constants::kZeroV3) > 0) {
                        nanoem_mutable_model_morph_vertex_t *item =
                            nanoemMutableModelMorphVertexCreate(parentMorph, &status);
                        nanoemMutableModelMorphVertexSetVertexObject(item, vertexPtr);
                        nanoemMutableModelMorphVertexSetPosition(item, glm::value_ptr(Vector4(p, 1)));
                        nanoemMutableModelMorphInsertVertexMorphObject(parentMorph, item, -1, &status);
                        nanoemMutableModelMorphVertexDestroy(item);
                        added.insert(vertexPtr);
                    }
                }
            }
        }
    }
    if (!added.empty()) {
        nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
        String name;
        nanoem_rsize_t numObjects;
        nanoemModelGetAllMorphObjects(opaque, &numObjects);
        StringUtils::format(name, "Morph%jd", numObjects + 1);
        StringUtils::UnicodeStringScope s(factory);
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
        }
        if (StringUtils::tryGetString(factory, name, s)) {
            nanoemMutableModelMorphSetName(parentMorph, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
        }
        ModelScope scope(m_model);
        nanoemMutableModelInsertMorphObject(scope.m_value, parentMorph, -1, &status);
        bind(parentMorph);
    }
    nanoemMutableModelMorphDestroy(parentMorph);
    handleStatus(status);
}

void
EditingModelTrait::Morph::createLabel(const model::Morph::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_label_t *label = nanoemMutableModelLabelCreate(m_model->data(), &status);
    const model::Morph::MutableList &bones = sortedAscent(value);
    for (model::Morph::MutableList::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
        nanoem_mutable_model_label_item_t *item = nanoemMutableModelLabelItemCreateFromMorphObject(label, *it, &status);
        nanoemMutableModelLabelInsertItemObject(label, item, -1, &status);
        nanoemMutableModelLabelItemDestroy(item);
    }
    nanoemMutableModelInsertLabelObject(scope.m_value, label, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Label%d",
        nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(nanoemMutableModelLabelGetOriginObject(label))));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    handleStatus(status);
}

int
EditingModelTrait::Morph::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::Morph::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::Morph::compareCommon(const void *left, const void *right)
{
    const nanoem_model_morph_t *lvalue = *static_cast<nanoem_model_morph_t *const *>(left);
    const nanoem_model_morph_t *rvalue = *static_cast<nanoem_model_morph_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(rvalue));
}

void
EditingModelTrait::Morph::moveItemAt(const model::Morph::MutableSet &value, int offset)
{
    model::Morph::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Morph::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_morph_t *item = nanoemMutableModelMorphCreateAsReference(*it, &status);
        nanoemMutableModelRemoveMorphObject(scope.m_value, item, &status);
        nanoemMutableModelInsertMorphObject(scope.m_value, item, offset, &status);
        nanoemMutableModelMorphDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Morph::bind(nanoem_mutable_model_morph_t *morphPtr)
{
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    model::Morph *morph = model::Morph::create();
    nanoem_model_morph_t *originPtr = nanoemMutableModelMorphGetOriginObject(morphPtr);
    morph->bind(originPtr);
    morph->resetLanguage(originPtr, factory, project->castLanguage());
}

model::Label::MutableList
EditingModelTrait::Label::sortedAscent(const model::Label::MutableSet &value)
{
    model::Label::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::Label::MutableList
EditingModelTrait::Label::sortedDescent(const model::Label::MutableSet &value)
{
    model::Label::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::Label::Label(Model *model)
    : Base(model)
{
}

EditingModelTrait::Label::~Label() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Label::addItem()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_label_t *label =
        nanoemMutableModelLabelCreate(nanoemMutableModelGetOriginObject(scope.m_value), &status);
    nanoem_model_label_t *origin = nanoemMutableModelLabelGetOriginObject(label);
    nanoemMutableModelInsertLabelObject(scope.m_value, label, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Label%d", nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(origin)));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelLabelSetName(label, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelLabelDestroy(label);
    handleStatus(status);
}

void
EditingModelTrait::Label::removeItem(const model::Label::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Label::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_label_t *item = nanoemMutableModelLabelCreateAsReference(*it, &status);
        nanoemMutableModelRemoveLabelObject(scope.m_value, item, &status);
        nanoemMutableModelLabelDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Label::moveItemTop(const model::Label::MutableSet &value)
{
    /* preserve root and expression label */
    moveItemAt(value, 2);
}

void
EditingModelTrait::Label::moveItemUp(const model::Label::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Label::moveItemDown(const model::Label::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Label::moveItemBottom(const model::Label::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::Label::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::Label::MutableList, TinySTLAllocator> NamedLabelList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(m_opaque, &numObjects);
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    NamedLabelList namedLabels;
    String name;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_label_t *labelPtr = labels[i];
        StringUtils::getUtf8String(nanoemModelLabelGetName(labelPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, name);
        namedLabels[name].push_back(labelPtr);
    }
    for (NamedLabelList::const_iterator it = namedLabels.begin(), end = namedLabels.end(); it != end; ++it) {
        const model::Label::MutableList &m = it->second;
        if (m.size() > 1) {
            model::Label::MutableSet set;
            for (model::Label::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2; ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

int
EditingModelTrait::Label::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::Label::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::Label::compareCommon(const void *left, const void *right)
{
    const nanoem_model_label_t *lvalue = *static_cast<nanoem_model_label_t *const *>(left);
    const nanoem_model_label_t *rvalue = *static_cast<nanoem_model_label_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(rvalue));
}

void
EditingModelTrait::Label::moveItemAt(const model::Label::MutableSet &value, int offset)
{
    model::Label::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Label::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_label_t *item = nanoemMutableModelLabelCreateAsReference(*it, &status);
        nanoemMutableModelRemoveLabelObject(scope.m_value, item, &status);
        nanoemMutableModelInsertLabelObject(scope.m_value, item, offset, &status);
        nanoemMutableModelLabelDestroy(item);
    }
    handleStatus(status);
}

model::RigidBody::MutableList
EditingModelTrait::RigidBody::sortedAscent(const model::RigidBody::MutableSet &value)
{
    model::RigidBody::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::RigidBody::MutableList
EditingModelTrait::RigidBody::sortedDescent(const model::RigidBody::MutableSet &value)
{
    model::RigidBody::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::RigidBody::RigidBody(Model *model)
    : Base(model)
{
}

EditingModelTrait::RigidBody::~RigidBody() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::RigidBody::addItem()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_rigid_body_t *rigidBody =
        nanoemMutableModelRigidBodyCreate(nanoemMutableModelGetOriginObject(scope.m_value), &status);
    nanoem_model_rigid_body_t *origin = nanoemMutableModelRigidBodyGetOriginObject(rigidBody);
    nanoemMutableModelInsertRigidBodyObject(scope.m_value, rigidBody, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "RigidBody%d", nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(origin)));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelRigidBodySetName(rigidBody, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelRigidBodySetName(rigidBody, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    PhysicsEngine *physics = project->physicsEngine();
    model::RigidBody *m = model::RigidBody::create();
    model::RigidBody::Resolver resolver;
    m->bind(origin, physics, false, resolver);
    m->resetLanguage(origin, factory, project->castLanguage());
    nanoemMutableModelRigidBodyDestroy(rigidBody);
    handleStatus(status);
}

void
EditingModelTrait::RigidBody::removeItem(const model::RigidBody::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::RigidBody::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_rigid_body_t *item = nanoemMutableModelRigidBodyCreateAsReference(*it, &status);
        nanoemMutableModelRemoveRigidBodyObject(scope.m_value, item, &status);
        nanoemMutableModelRigidBodyDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::RigidBody::moveItemTop(const model::RigidBody::MutableSet &value)
{
    moveItemAt(value, 0);
}

void
EditingModelTrait::RigidBody::moveItemUp(const model::RigidBody::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::RigidBody::moveItemDown(const model::RigidBody::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::RigidBody::moveItemBottom(const model::RigidBody::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::RigidBody::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::RigidBody::MutableList, TinySTLAllocator> NamedRigidBodyList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_rigid_body_t *const *rigid_bodys = nanoemModelGetAllRigidBodyObjects(m_opaque, &numObjects);
    NamedRigidBodyList namedRigidBodys;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_rigid_body_t *bodyPtr = rigid_bodys[i];
        if (const model::RigidBody *rigid_body = model::RigidBody::cast(bodyPtr)) {
            namedRigidBodys[rigid_body->canonicalName()].push_back(bodyPtr);
        }
    }
    for (NamedRigidBodyList::const_iterator it = namedRigidBodys.begin(), end = namedRigidBodys.end(); it != end;
         ++it) {
        const model::RigidBody::MutableList &m = it->second;
        if (m.size() > 1) {
            model::RigidBody::MutableSet set;
            for (model::RigidBody::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2;
                 ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

void
EditingModelTrait::RigidBody::createRigidBodyFromBone(const nanoem_model_bone_t *value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_mutable_model_rigid_body_t *body = nanoemMutableModelRigidBodyCreate(opaque, &status);
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    String name;
    nanoem_rsize_t numObjects;
    nanoemModelGetAllRigidBodyObjects(opaque, &numObjects);
    StringUtils::format(name, "RigidBody%jd", numObjects + 1);
    StringUtils::UnicodeStringScope s(factory);
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelRigidBodySetName(body, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelRigidBodySetName(body, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelRigidBodySetBoneObject(body, value);
    nanoemMutableModelRigidBodySetShapeSize(body, glm::value_ptr(Vector4(1)));
    ModelScope scope(m_model);
    nanoemMutableModelInsertRigidBodyObject(scope.m_value, body, -1, &status);
    nanoemMutableModelRigidBodyDestroy(body);
    handleStatus(status);
}

int
EditingModelTrait::RigidBody::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::RigidBody::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::RigidBody::compareCommon(const void *left, const void *right)
{
    const nanoem_model_rigid_body_t *lvalue = *static_cast<nanoem_model_rigid_body_t *const *>(left);
    const nanoem_model_rigid_body_t *rvalue = *static_cast<nanoem_model_rigid_body_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(rvalue));
}

void
EditingModelTrait::RigidBody::moveItemAt(const model::RigidBody::MutableSet &value, int offset)
{
    model::RigidBody::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::RigidBody::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_rigid_body_t *item = nanoemMutableModelRigidBodyCreateAsReference(*it, &status);
        nanoemMutableModelRemoveRigidBodyObject(scope.m_value, item, &status);
        nanoemMutableModelInsertRigidBodyObject(scope.m_value, item, offset, &status);
        nanoemMutableModelRigidBodyDestroy(item);
    }
    handleStatus(status);
}

model::Joint::MutableList
EditingModelTrait::Joint::sortedAscent(const model::Joint::MutableSet &value)
{
    model::Joint::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareAscent);
    return list;
}

model::Joint::MutableList
EditingModelTrait::Joint::sortedDescent(const model::Joint::MutableSet &value)
{
    model::Joint::MutableList list(ListUtils::toListFromSet(value));
    qsort(list.data(), list.size(), sizeof(list[0]), compareDescent);
    return list;
}

EditingModelTrait::Joint::Joint(Model *model)
    : Base(model)
{
}

EditingModelTrait::Joint::~Joint() NANOEM_DECL_NOEXCEPT
{
}

void
EditingModelTrait::Joint::addItem()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    nanoem_mutable_model_joint_t *joint =
        nanoemMutableModelJointCreate(nanoemMutableModelGetOriginObject(scope.m_value), &status);
    nanoem_model_joint_t *origin = nanoemMutableModelJointGetOriginObject(joint);
    nanoemMutableModelInsertJointObject(scope.m_value, joint, -1, &status);
    Project *project = m_model->project();
    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
    StringUtils::UnicodeStringScope s(factory);
    String name;
    StringUtils::format(name, "Joint%d", nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(origin)));
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelJointSetName(joint, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelJointSetName(joint, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    PhysicsEngine *physics = project->physicsEngine();
    model::RigidBody::Resolver resolver;
    model::Joint *m = model::Joint::create();
    m->bind(origin, physics, resolver);
    m->resetLanguage(origin, factory, project->castLanguage());
    nanoemMutableModelJointDestroy(joint);
    handleStatus(status);
}

void
EditingModelTrait::Joint::removeItem(const model::Joint::MutableSet &value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Joint::MutableSet::const_iterator it = value.begin(), end = value.end(); it != end; ++it) {
        nanoem_mutable_model_joint_t *item = nanoemMutableModelJointCreateAsReference(*it, &status);
        nanoemMutableModelRemoveJointObject(scope.m_value, item, &status);
        nanoemMutableModelJointDestroy(item);
    }
    handleStatus(status);
}

void
EditingModelTrait::Joint::moveItemTop(const model::Joint::MutableSet &value)
{
    moveItemAt(value, 0);
}

void
EditingModelTrait::Joint::moveItemUp(const model::Joint::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Joint::moveItemDown(const model::Joint::MutableSet &value)
{
    BX_UNUSED_1(value);
}

void
EditingModelTrait::Joint::moveItemBottom(const model::Joint::MutableSet &value)
{
    moveItemAt(value, -1);
}

void
EditingModelTrait::Joint::mergeAllDuplicates()
{
    typedef tinystl::unordered_map<String, model::Joint::MutableList, TinySTLAllocator> NamedJointList;
    nanoem_model_t *m_opaque = m_model->data();
    nanoem_rsize_t numObjects;
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(m_opaque, &numObjects);
    NamedJointList namedJoints;
    for (nanoem_rsize_t i = 0; i < numObjects; i++) {
        nanoem_model_joint_t *jointPtr = joints[i];
        if (const model::Joint *joint = model::Joint::cast(jointPtr)) {
            namedJoints[joint->canonicalName()].push_back(jointPtr);
        }
    }
    for (NamedJointList::const_iterator it = namedJoints.begin(), end = namedJoints.end(); it != end; ++it) {
        const model::Joint::MutableList &m = it->second;
        if (m.size() > 1) {
            model::Joint::MutableSet set;
            for (model::Joint::MutableList::const_iterator it2 = m.begin() + 1, end2 = m.end(); it2 != end2; ++it2) {
                set.insert(*it2);
            }
            removeItem(set);
        }
    }
}

void
EditingModelTrait::Joint::createJointFromRigidBody(
    const nanoem_model_rigid_body_t *a, const nanoem_model_rigid_body_t *b)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_model_t *opaque = m_model->data();
    nanoem_mutable_model_joint_t *joint = nanoemMutableModelJointCreate(opaque, &status);
    nanoem_unicode_string_factory_t *factory = m_model->project()->unicodeStringFactory();
    String name;
    nanoem_rsize_t numObjects;
    nanoemModelGetAllRigidBodyObjects(opaque, &numObjects);
    StringUtils::format(name, "Joint%jd", numObjects + 1);
    StringUtils::UnicodeStringScope s(factory);
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelJointSetName(joint, s.value(), NANOEM_LANGUAGE_TYPE_JAPANESE, &status);
    }
    if (StringUtils::tryGetString(factory, name, s)) {
        nanoemMutableModelJointSetName(joint, s.value(), NANOEM_LANGUAGE_TYPE_ENGLISH, &status);
    }
    nanoemMutableModelJointSetRigidBodyAObject(joint, a);
    nanoemMutableModelJointSetRigidBodyBObject(joint, b);
    ModelScope scope(m_model);
    nanoemMutableModelInsertJointObject(scope.m_value, joint, -1, &status);
    nanoemMutableModelJointDestroy(joint);
    handleStatus(status);
}

int
EditingModelTrait::Joint::compareAscent(const void *left, const void *right)
{
    return compareCommon(left, right);
}

int
EditingModelTrait::Joint::compareDescent(const void *left, const void *right)
{
    return compareCommon(right, left);
}

int
EditingModelTrait::Joint::compareCommon(const void *left, const void *right)
{
    const nanoem_model_joint_t *lvalue = *static_cast<nanoem_model_joint_t *const *>(left);
    const nanoem_model_joint_t *rvalue = *static_cast<nanoem_model_joint_t *const *>(right);
    return nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(lvalue)) -
        nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(rvalue));
}

void
EditingModelTrait::Joint::moveItemAt(const model::Joint::MutableSet &value, int offset)
{
    model::Joint::MutableList l = sortedDescent(value);
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    ModelScope scope(m_model);
    for (model::Joint::MutableList::const_iterator it = l.begin(), end = l.end(); it != end; ++it) {
        nanoem_mutable_model_joint_t *item = nanoemMutableModelJointCreateAsReference(*it, &status);
        nanoemMutableModelRemoveJointObject(scope.m_value, item, &status);
        nanoemMutableModelInsertJointObject(scope.m_value, item, offset, &status);
        nanoemMutableModelJointDestroy(item);
    }
    handleStatus(status);
}

} /* namespace internal */
} /* namespace nanoem */
