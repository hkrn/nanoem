/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/model/Validator.h"

#include "emapp/EnumUtils.h"
#include "emapp/IImageView.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/StringUtils.h"
#include "emapp/model/Joint.h"
#include "emapp/model/Label.h"
#include "emapp/model/SoftBody.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace model {
namespace {

class PrivateUtils : private NonCopyable {
public:
    static int
    index(const nanoem_model_vertex_t *vertexPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(vertexPtr));
    }
    static int
    index(const nanoem_model_material_t *materialPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(materialPtr));
    }
    static int
    index(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(bonePtr));
    }
    static int
    index(const nanoem_model_morph_t *morphPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(morphPtr));
    }
    static int
    index(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(labelPtr));
    }
    static int
    index(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(rigidBodyPtr));
    }
    static int
    index(const nanoem_model_joint_t *jointPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(jointPtr));
    }
    static int
    index(const nanoem_model_soft_body_t *softBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        return nanoemModelObjectGetIndex(nanoemModelSoftBodyGetModelObject(softBodyPtr));
    }
    static const char *
    nameConstString(const nanoem_model_material_t *materialPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Material *material = model::Material::cast(materialPtr);
        return material ? material->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Bone *bone = model::Bone::cast(bonePtr);
        return bone ? bone->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_morph_t *morphPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Morph *morph = model::Morph::cast(morphPtr);
        return morph ? morph->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Label *label = model::Label::cast(labelPtr);
        return label ? label->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
        return rigidBody ? rigidBody->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_joint_t *jointPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Joint *joint = model::Joint::cast(jointPtr);
        return joint ? joint->nameConstString() : "(unknown)";
    }
    static const char *
    nameConstString(const nanoem_model_soft_body_t *softBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr);
        return softBody ? softBody->nameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_material_t *materialPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Material *material = model::Material::cast(materialPtr);
        return material ? material->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Bone *bone = model::Bone::cast(bonePtr);
        return bone ? bone->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_morph_t *morphPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Morph *morph = model::Morph::cast(morphPtr);
        return morph ? morph->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_label_t *labelPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Label *label = model::Label::cast(labelPtr);
        return label ? label->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_rigid_body_t *rigidBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::RigidBody *rigidBody = model::RigidBody::cast(rigidBodyPtr);
        return rigidBody ? rigidBody->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_joint_t *jointPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::Joint *joint = model::Joint::cast(jointPtr);
        return joint ? joint->canonicalNameConstString() : "(unknown)";
    }
    static const char *
    canonicalNameConstString(const nanoem_model_soft_body_t *softBodyPtr) NANOEM_DECL_NOEXCEPT
    {
        const model::SoftBody *softBody = model::SoftBody::cast(softBodyPtr);
        return softBody ? softBody->canonicalNameConstString() : "(unknown)";
    }
};

} /* anonymous */

Validator::Validator()
{
}

Validator::~Validator() NANOEM_DECL_NOEXCEPT
{
}

void
Validator::validate(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    validateAllVertexObjects(model, filter, result);
    validateAllFaces(model, filter, result);
    validateAllMaterialObjects(model, filter, result);
    validateAllBoneObjects(model, filter, result);
    validateAllLabelObjects(model, filter, result);
    validateAllMorphObjects(model, filter, result);
    validateAllRigidBodyObjects(model, filter, result);
    validateAllJointObjects(model, filter, result);
    validateAllSoftBodyObjects(model, filter, result);
}

void
Validator::format(const Diagnostics &diag, const ITranslator *translator, String &text)
{
    switch (diag.m_message) {
    case kMessageTypePrimitiveFloatNaN: {
        StringUtils::format(text, "* %s\n", translator->translate("nanoem.model.validator.primitive.float-nan"));
        break;
    }
    case kMessageTypePrimitiveFloatInfinity: {
        StringUtils::format(text, "* %s\n", translator->translate("nanoem.model.validator.primitive.float-inf"));
        break;
    }
    case kMessageTypeVertexNormalInvalid: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.vertex.normal.invalid"),
            PrivateUtils::index(diag.u.m_vertexPtr));
        break;
    }
    case kMessageTypeVertexTexCoordOutOfBound: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.vertex.texcoord.oob"),
            PrivateUtils::index(diag.u.m_vertexPtr));
        break;
    }
    case kMessageTypeVertexNullBoneObject: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.vertex.bone.null"),
            PrivateUtils::index(diag.u.m_vertexPtr));
        break;
    }
    case kMessageTypeVertexBoneWeightNotNormalized: {
        StringUtils::format(text, "* %s: %d\n",
            translator->translate("nanoem.model.validator.vertex.weight.not-normalized"),
            PrivateUtils::index(diag.u.m_vertexPtr));
        break;
    }
    case kMessageTypeVertexInvalidType: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.vertex.type.invalid"),
            PrivateUtils::index(diag.u.m_vertexPtr));
        break;
    }
    case kMessageTypeVertexIndexNotTriangulated: {
        StringUtils::format(text, "* %s\n", translator->translate("nanoem.model.validator.face.not-triangulated"));
        break;
    }
    case kMessageTypeVertexIndexNullVertexObject: {
        StringUtils::format(text, "* %s\n", translator->translate("nanoem.model.validator.face.vertex.null"));
        break;
    }
    case kMessageTypeVertexIndexOutOfBound: {
        StringUtils::format(text, "* %s\n", translator->translate("nanoem.model.validator.face.oob"));
        break;
    }
    case kMessageTypeMaterialEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.material.name.empty"),
            PrivateUtils::index(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.material.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialAmbientColorOutOfBound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.ambient.color.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialDiffuseColorOutOfBound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.diffuse.color.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialDiffuseOpacityOutOfBound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.diffuse.opacity.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialSpecularColorOutOfBound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.specular.color.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialEdgeColorOutOfBound: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.material.edge.color.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialEdgeOpacityOutOfBound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.edge.opacity.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialDiffuseTextureNotFound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.texture.diffuse.not-found"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialSphereMapTextureNotFound: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.material.texture.sphere-map.not-found"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialToonTextureNotFound: {
        StringUtils::format(text, "* %s:%s\n",
            translator->translate("nanoem.model.validator.material.texture.toon.not-found"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeMaterialVertexIndexOutOfBound: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.material.face.oob"),
            PrivateUtils::nameConstString(diag.u.m_materialPtr));
        break;
    }
    case kMessageTypeBoneTooLongName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.bone.name.too-long"),
            PrivateUtils::canonicalNameConstString(diag.u.m_bonePtr));
        break;
    }
    case kMessageTypeBoneEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.bone.name.empty"),
            PrivateUtils::index(diag.u.m_bonePtr));
        break;
    }
    case kMessageTypeBoneDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.bone.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_bonePtr));
        break;
    }
    case kMessageTypeBoneInherentBoneNullBoneObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.bone.inherent.null"),
            PrivateUtils::nameConstString(diag.u.m_bonePtr));
        break;
    }
    case kMessageTypeBoneFixedAxisNotNormalized: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.bone.fixed-axis.not-normalized"),
            PrivateUtils::nameConstString(diag.u.m_bonePtr));
        break;
    }
    case kMessageTypeMorphTooLongName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.morph.name.too-long"),
            PrivateUtils::canonicalNameConstString(diag.u.m_morphPtr));
        break;
    }
    case kMessageTypeMorphEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.morph.name.empty"),
            PrivateUtils::index(diag.u.m_morphPtr));
        break;
    }
    case kMessageTypeMorphDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.morph.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_morphPtr));
        break;
    }
    case kMessageTypeLabelEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.label.name.empty"),
            PrivateUtils::index(diag.u.m_labelPtr));
        break;
    }
    case kMessageTypeLabelDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n",
            translator->translate("nanoem.model.validator.label.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_labelPtr));
        break;
    }
    case kMessageTypeLabelEmptyItems: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.label.empty"),
            PrivateUtils::nameConstString(diag.u.m_labelPtr));
        break;
    }
    case kMessageTypeLabelItemNullBoneObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.label.item.bone.null"),
            PrivateUtils::nameConstString(diag.u.m_labelPtr));
        break;
    }
    case kMessageTypeLabelItemNullMorphObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.label.item.morph.null"),
            PrivateUtils::nameConstString(diag.u.m_labelPtr));
        break;
    }
    case kMessageTypeRigidBodyEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.rigid-body.name.empty"),
            PrivateUtils::index(diag.u.m_rigidBodyPtr));
        break;
    }
    case kMessageTypeRigidBodyDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.rigid-body.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_rigidBodyPtr));
        break;
    }
    case kMessageTypeRigidBodyNullBoneObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.rigid-body.bone.null"),
            PrivateUtils::nameConstString(diag.u.m_rigidBodyPtr));
        break;
    }
    case kMessageTypeJointEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.joint.name.empty"),
            PrivateUtils::index(diag.u.m_jointPtr));
        break;
    }
    case kMessageTypeJointDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.joint.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_jointPtr));
        break;
    }
    case kMessageTypeJointRigidBodyANullObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.joint.rigid-body-a.null"),
            PrivateUtils::nameConstString(diag.u.m_jointPtr));
        break;
    }
    case kMessageTypeJointRigidBodyBNullObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.joint.rigid-body-b.null"),
            PrivateUtils::nameConstString(diag.u.m_jointPtr));
        break;
    }
    case kMessageTypeSoftBodyEmptyName: {
        StringUtils::format(text, "* %s: %d\n", translator->translate("nanoem.model.validator.soft-body.name.empty"),
            PrivateUtils::index(diag.u.m_softBodyPtr));
        break;
    }
    case kMessageTypeSoftBodyDuplicatedName: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.soft-body.name.duplicated"),
            PrivateUtils::canonicalNameConstString(diag.u.m_softBodyPtr));
        break;
    }
    case kMessageTypeSoftBodyNullMaterialObject: {
        StringUtils::format(text, "* %s: %s\n", translator->translate("nanoem.model.validator.soft-body.material.null"),
            PrivateUtils::nameConstString(diag.u.m_softBodyPtr));
        break;
    }
    default:
        break;
    }
}

void
Validator::validateAllVertexObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numVertices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    Diagnostics diag;
    for (nanoem_rsize_t i = 0; i < numVertices; i++) {
        const nanoem_model_vertex_t *vertexPtr = vertices[i];
        diag.u.m_vertexPtr = vertexPtr;
        if (!validateVector3(nanoemModelVertexGetOrigin(vertexPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateNormal(nanoemModelVertexGetNormal(vertexPtr), kMessageTypeVertexNormalInvalid, filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateTexCoord(
                nanoemModelVertexGetTexCoord(vertexPtr), kMessageTypeVertexTexCoordOutOfBound, filter, &diag)) {
            result.push_back(diag);
        }
        switch (nanoemModelVertexGetType(vertexPtr)) {
        case NANOEM_MODEL_VERTEX_TYPE_BDEF1: {
            if (nanoemModelVertexGetBoneObject(vertexPtr, 0) == nullptr &&
                testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                diag.m_message = kMessageTypeVertexNullBoneObject;
                result.push_back(diag);
            }
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_BDEF2:
        case NANOEM_MODEL_VERTEX_TYPE_SDEF: {
            for (nanoem_rsize_t i = 0; i < 2; i++) {
                if (nanoemModelVertexGetBoneObject(vertexPtr, i) == nullptr &&
                    testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                    diag.m_message = kMessageTypeVertexNullBoneObject;
                    result.push_back(diag);
                }
            }
            const nanoem_f32_t weight = nanoemModelVertexGetBoneWeight(vertexPtr, 0);
            if ((weight < 0.0f || weight > 1.0f) && testDiagnosticsSeverity(kSeverityTypeWarning, filter, &diag)) {
                diag.m_message = kMessageTypeVertexBoneWeightNotNormalized;
                result.push_back(diag);
            }
            break;
        }
        case NANOEM_MODEL_VERTEX_TYPE_BDEF4:
        case NANOEM_MODEL_VERTEX_TYPE_QDEF: {
            nanoem_f32_t weight = 0.0f;
            for (nanoem_rsize_t i = 0; i < 4; i++) {
                if (nanoemModelVertexGetBoneObject(vertexPtr, i) == nullptr &&
                    testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                    diag.m_message = kMessageTypeVertexNullBoneObject;
                    result.push_back(diag);
                }
                weight += nanoemModelVertexGetBoneWeight(vertexPtr, i);
            }
            if (!glm::epsilonEqual(weight, 1.0f, Constants::kEpsilon) &&
                testDiagnosticsSeverity(kSeverityTypeWarning, filter, &diag)) {
                diag.m_message = kMessageTypeVertexBoneWeightNotNormalized;
                result.push_back(diag);
            }
            break;
        }
        default: {
            if (testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                diag.m_message = kMessageTypeVertexInvalidType;
                result.push_back(diag);
            }
            break;
        }
        }
    }
}

void
Validator::validateAllFaces(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numVertices, numIndices;
    nanoem_model_vertex_t *const *vertices = nanoemModelGetAllVertexObjects(model->data(), &numVertices);
    const nanoem_u32_t *indices = nanoemModelGetAllVertexIndices(model->data(), &numIndices);
    Diagnostics diag;
    diag.u.m_bonePtr = nullptr;
    if (numIndices % 3 != 0 && testDiagnosticsSeverity(kSeverityTypeFatal, filter, &diag)) {
        diag.m_message = kMessageTypeVertexIndexNotTriangulated;
        result.push_back(diag);
    }
    else {
        for (nanoem_rsize_t i = 0; i < numIndices; i += 3) {
            for (nanoem_rsize_t j = 0; j < 3; j++) {
                nanoem_u32_t offset = indices[i + j];
                if (offset < numVertices) {
                    const nanoem_model_vertex_t *vertexPtr = vertices[offset];
                    if (vertexPtr == nullptr && testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                        diag.m_message = kMessageTypeVertexIndexNullVertexObject;
                        result.push_back(diag);
                    }
                }
                else if (testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
                    diag.m_message = kMessageTypeVertexIndexOutOfBound;
                    result.push_back(diag);
                }
            }
        }
    }
}

void
Validator::validateAllMaterialObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numMaterials;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
    StringSet nameSet;
    String utf8Name;
    Diagnostics diag;
    for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
        const nanoem_model_material_t *materialPtr = materials[i];
        const nanoem_unicode_string_t *name = nanoemModelMaterialGetName(materialPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_materialPtr = materialPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeMaterialEmptyName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        else if (!nameSet.insert(utf8Name).second) {
            diag.m_message = kMessageTypeMaterialDuplicatedName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        if (!validateColor(nanoemModelMaterialGetAmbientColor(materialPtr), kMessageTypeMaterialAmbientColorOutOfBound,
                filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateColor(nanoemModelMaterialGetDiffuseColor(materialPtr), kMessageTypeMaterialDiffuseColorOutOfBound,
                filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateColor(nanoemModelMaterialGetSpecularColor(materialPtr),
                kMessageTypeMaterialSpecularColorOutOfBound, filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateColor(
                nanoemModelMaterialGetEdgeColor(materialPtr), kMessageTypeMaterialEdgeColorOutOfBound, filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateOpacity(nanoemModelMaterialGetDiffuseOpacity(materialPtr),
                kMessageTypeMaterialDiffuseOpacityOutOfBound, filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateOpacity(nanoemModelMaterialGetEdgeOpacity(materialPtr), kMessageTypeMaterialEdgeOpacityOutOfBound,
                filter, &diag)) {
            result.push_back(diag);
        }
        if (const model::Material *material = model::Material::cast(materialPtr)) {
            if (const IImageView *view = material->diffuseImage()) {
                if (!view->isFileExist() && testDiagnosticsSeverity(kSeverityTypeWarning, filter, &diag)) {
                    diag.m_message = kMessageTypeMaterialDiffuseTextureNotFound;
                    result.push_back(diag);
                }
            }
            if (const IImageView *view = material->sphereMapImage()) {
                if (!view->isFileExist() && testDiagnosticsSeverity(kSeverityTypeWarning, filter, &diag)) {
                    diag.m_message = kMessageTypeMaterialSphereMapTextureNotFound;
                    result.push_back(diag);
                }
            }
            if (const IImageView *view = material->toonImage()) {
                if (!view->isFileExist() && testDiagnosticsSeverity(kSeverityTypeWarning, filter, &diag)) {
                    diag.m_message = kMessageTypeMaterialToonTextureNotFound;
                    result.push_back(diag);
                }
            }
        }
    }
}

void
Validator::validateAllBoneObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numBones;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
    StringUtils::UnicodeStringScope scope(factory);
    StringSet nameSet;
    String utf8Name;
    Diagnostics diag;
    for (nanoem_rsize_t i = 0; i < numBones; i++) {
        const nanoem_model_bone_t *bonePtr = bones[i];
        const nanoem_unicode_string_t *name = nanoemModelBoneGetName(bonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_bonePtr = bonePtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeBoneEmptyName;
            diag.m_severity = kSeverityTypeError;
            result.push_back(diag);
        }
        else {
            if (!validateNameInShiftJIS(name, 15, kMessageTypeBoneTooLongName, filter, factory, &diag)) {
                bool selectable = model::Bone::isSelectable(bonePtr),
                     canPush = selectable || (!selectable && testDiagnosticsSeverity(kSeverityTypeInfo, filter, &diag));
                if (canPush) {
                    result.push_back(diag);
                }
            }
            if (!nameSet.insert(utf8Name).second) {
                diag.m_message = kMessageTypeBoneDuplicatedName;
                diag.m_severity = kSeverityTypeWarning;
                result.push_back(diag);
            }
        }
        if (!validateVector3(nanoemModelBoneGetOrigin(bonePtr), filter, &diag)) {
            result.push_back(diag);
        }
    }
}

void
Validator::validateAllMorphObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numMorphs;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_morph_t *const *morphs = nanoemModelGetAllMorphObjects(model->data(), &numMorphs);
    StringUtils::UnicodeStringScope scope(factory);
    StringSet nameSet;
    String utf8Name;
    Diagnostics diag;
    for (nanoem_rsize_t i = 0; i < numMorphs; i++) {
        const nanoem_model_morph_t *morphPtr = morphs[i];
        const nanoem_unicode_string_t *name = nanoemModelMorphGetName(morphPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_morphPtr = morphPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeMorphEmptyName;
            diag.m_severity = kSeverityTypeError;
            result.push_back(diag);
        }
        else {
            if (!validateNameInShiftJIS(name, 15, kMessageTypeMorphTooLongName, filter, factory, &diag)) {
                bool isHidden = nanoemModelMorphGetCategory(morphPtr) == NANOEM_MODEL_MORPH_CATEGORY_BASE,
                     canPush = !isHidden || (isHidden && testDiagnosticsSeverity(kSeverityTypeInfo, filter, &diag));
                if (canPush) {
                    result.push_back(diag);
                }
            }
            if (!nameSet.insert(utf8Name).second) {
                diag.m_message = kMessageTypeMorphDuplicatedName;
                diag.m_severity = kSeverityTypeWarning;
                result.push_back(diag);
            }
        }
    }
}

void
Validator::validateAllLabelObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numLabels, numItems;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_label_t *const *labels = nanoemModelGetAllLabelObjects(model->data(), &numLabels);
    Diagnostics diag;
    StringSet nameSet;
    String utf8Name;
    for (nanoem_rsize_t i = 0; i < numLabels; i++) {
        const nanoem_model_label_t *labelPtr = labels[i];
        const nanoem_unicode_string_t *name = nanoemModelLabelGetName(labelPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_labelPtr = labelPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeLabelEmptyName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        else if (!nameSet.insert(utf8Name).second) {
            diag.m_message = kMessageTypeLabelDuplicatedName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        nanoem_model_label_item_t *const *items = nanoemModelLabelGetAllItemObjects(labelPtr, &numItems);
        if (numItems == 0 && testDiagnosticsSeverity(kSeverityTypeInfo, filter, &diag)) {
            diag.m_message = kMessageTypeLabelEmptyItems;
            result.push_back(diag);
        }
        for (nanoem_rsize_t j = 0; j < numItems; j++) {
            const nanoem_model_label_item_t *item = items[j];
            diag.u.m_labelItemPtr = item;
            switch (nanoemModelLabelItemGetType(item)) {
            case NANOEM_MODEL_LABEL_ITEM_TYPE_BONE: {
                if (nanoemModelLabelItemGetBoneObject(item) == nullptr) {
                    diag.m_message = kMessageTypeLabelItemNullBoneObject;
                    diag.m_severity = kSeverityTypeError;
                    result.push_back(diag);
                }
                break;
            }
            case NANOEM_MODEL_LABEL_ITEM_TYPE_MORPH: {
                if (nanoemModelLabelItemGetMorphObject(item) == nullptr) {
                    diag.m_message = kMessageTypeLabelItemNullMorphObject;
                    diag.m_severity = kSeverityTypeError;
                    result.push_back(diag);
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

void
Validator::validateAllRigidBodyObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numRigidBodies;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_rigid_body_t *const *rigidBodies = nanoemModelGetAllRigidBodyObjects(model->data(), &numRigidBodies);
    Diagnostics diag;
    StringSet nameSet;
    String utf8Name;
    for (nanoem_rsize_t i = 0; i < numRigidBodies; i++) {
        const nanoem_model_rigid_body_t *rigidBodyPtr = rigidBodies[i];
        const nanoem_unicode_string_t *name = nanoemModelRigidBodyGetName(rigidBodyPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_rigidBodyPtr = rigidBodyPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeRigidBodyEmptyName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        else if (!nameSet.insert(utf8Name).second) {
            diag.m_message = kMessageTypeRigidBodyDuplicatedName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelRigidBodyGetOrigin(rigidBodyPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelRigidBodyGetOrientation(rigidBodyPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (nanoemModelRigidBodyGetBoneObject(rigidBodyPtr) == nullptr &&
            testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
            diag.m_message = kMessageTypeRigidBodyNullBoneObject;
            result.push_back(diag);
        }
    }
}

void
Validator::validateAllJointObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numJoints;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_joint_t *const *joints = nanoemModelGetAllJointObjects(model->data(), &numJoints);
    Diagnostics diag;
    StringSet nameSet;
    String utf8Name;
    for (nanoem_rsize_t i = 0; i < numJoints; i++) {
        const nanoem_model_joint_t *jointPtr = joints[i];
        const nanoem_unicode_string_t *name = nanoemModelJointGetName(jointPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_jointPtr = jointPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeJointEmptyName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        else if (!nameSet.insert(utf8Name).second) {
            diag.m_message = kMessageTypeJointDuplicatedName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        if (nanoemModelJointGetRigidBodyAObject(jointPtr) == nullptr &&
            testDiagnosticsSeverity(kSeverityTypeFatal, filter, &diag)) {
            diag.m_message = kMessageTypeJointRigidBodyANullObject;
            result.push_back(diag);
        }
        if (nanoemModelJointGetRigidBodyBObject(jointPtr) == nullptr &&
            testDiagnosticsSeverity(kSeverityTypeFatal, filter, &diag)) {
            diag.m_message = kMessageTypeJointRigidBodyBNullObject;
            result.push_back(diag);
        }
        if (!validateVector3(nanoemModelJointGetOrigin(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateVector3(nanoemModelJointGetLinearUpperLimit(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateVector3(nanoemModelJointGetLinearLowerLimit(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateVector3(nanoemModelJointGetLinearStiffness(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelJointGetOrientation(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelJointGetAngularUpperLimit(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelJointGetAngularLowerLimit(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
        if (!validateEulerAngles(nanoemModelJointGetAngularStiffness(jointPtr), filter, &diag)) {
            result.push_back(diag);
        }
    }
}

void
Validator::validateAllSoftBodyObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result)
{
    nanoem_rsize_t numSoftBodies;
    nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
    nanoem_model_soft_body_t *const *softBodies = nanoemModelGetAllSoftBodyObjects(model->data(), &numSoftBodies);
    Diagnostics diag;
    StringSet nameSet;
    String utf8Name;
    for (nanoem_rsize_t i = 0; i < numSoftBodies; i++) {
        const nanoem_model_soft_body_t *softBodyPtr = softBodies[i];
        const nanoem_unicode_string_t *name = nanoemModelSoftBodyGetName(softBodyPtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        diag.u.m_softBodyPtr = softBodyPtr;
        StringUtils::getUtf8String(name, factory, utf8Name);
        if (utf8Name.empty()) {
            diag.m_message = kMessageTypeSoftBodyEmptyName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        else if (!nameSet.insert(utf8Name).second) {
            diag.m_message = kMessageTypeSoftBodyDuplicatedName;
            diag.m_severity = kSeverityTypeWarning;
            result.push_back(diag);
        }
        if (nanoemModelSoftBodyGetMaterialObject(softBodyPtr) == nullptr &&
            testDiagnosticsSeverity(kSeverityTypeError, filter, &diag)) {
            diag.m_message = kMessageTypeSoftBodyNullMaterialObject;
            result.push_back(diag);
        }
    }
}

bool
Validator::validateNameInShiftJIS(const nanoem_unicode_string_t *value, nanoem_rsize_t expected, MessageType type,
    nanoem_u32_t filter, nanoem_unicode_string_factory_t *factory, Diagnostics *diag)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t length = 0;
    diag->m_message = kMessageTypeNone;
    if (nanoem_u8_t *bytes =
            nanoemUnicodeStringFactoryGetByteArrayEncoding(factory, value, &length, NANOEM_CODEC_TYPE_SJIS, &status)) {
        nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    }
    if (length > expected && testDiagnosticsSeverity(kSeverityTypeWarning, filter, diag)) {
        diag->m_message = type;
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateVector3(const nanoem_f32_t *value, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    for (nanoem_rsize_t i = 0; i < 3; i++) {
        const float component = value[i];
        if (glm::isnan(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatNaN;
            break;
        }
        else if (glm::isinf(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatInfinity;
            break;
        }
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateNormal(
    const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    if (glm::abs(glm::length(glm::make_vec3(value)) < Constants::kEpsilon) &&
        testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
        diag->m_message = type;
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateTexCoord(
    const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    for (nanoem_rsize_t i = 0; i < 2; i++) {
        const float component = value[i];
        if (glm::isnan(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatNaN;
            break;
        }
        else if (glm::isinf(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatInfinity;
            break;
        }
        else if ((component < -1.0f || component > 1.0f) && testDiagnosticsSeverity(kSeverityTypeInfo, filter, diag)) {
            diag->m_message = type;
            break;
        }
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateEulerAngles(
    const nanoem_f32_t *value, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    for (nanoem_rsize_t i = 0; i < 3; i++) {
        const float component = value[i];
        if (glm::isnan(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatNaN;
            break;
        }
        else if (glm::isinf(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatInfinity;
            break;
        }
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateColor(
    const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    for (nanoem_rsize_t i = 0; i < 3; i++) {
        const float component = value[i];
        if (glm::isnan(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatNaN;
            break;
        }
        else if (glm::isinf(component) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
            diag->m_message = kMessageTypePrimitiveFloatInfinity;
            break;
        }
        else if ((component < 0.0f || component > 1.0f) &&
            testDiagnosticsSeverity(kSeverityTypeWarning, filter, diag)) {
            diag->m_message = type;
            break;
        }
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::validateOpacity(
    const nanoem_f32_t value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    diag->m_message = kMessageTypeNone;
    if (glm::isnan(value) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
        diag->m_message = kMessageTypePrimitiveFloatNaN;
    }
    else if (glm::isinf(value) && testDiagnosticsSeverity(kSeverityTypeError, filter, diag)) {
        diag->m_message = kMessageTypePrimitiveFloatInfinity;
    }
    else if ((value < 0.0f || value > 1.0f) && testDiagnosticsSeverity(kSeverityTypeWarning, filter, diag)) {
        diag->m_message = type;
    }
    return diag->m_message == kMessageTypeNone;
}

bool
Validator::testDiagnosticsSeverity(
    SeverityType severity, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT
{
    bool result = false;
    if (EnumUtils::isEnabled(severity, filter)) {
        diag->m_severity = severity;
        result = true;
    }
    return result;
}

} /* namespace model */
} /* namespace nanoem */
