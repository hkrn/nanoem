/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_VALIDATOR_H_
#define NANOEM_EMAPP_MODEL_VALIDATOR_H_

#include "emapp/Forward.h"

namespace nanoem {

class ITranslator;
class Model;

namespace model {

class Validator : private NonCopyable {
public:
    enum MessageType {
        kMessageTypeFirstEnum = 0,
        kMessageTypeNone = kMessageTypeFirstEnum,
        kMessageTypePrimitiveFloatNaN,
        kMessageTypePrimitiveFloatInfinity,
        kMessageTypeVertexNormalInvalid,
        kMessageTypeVertexTexCoordOutOfBound,
        kMessageTypeVertexNullBoneObject,
        kMessageTypeVertexBoneWeightNotNormalized,
        kMessageTypeVertexInvalidType,
        kMessageTypeFaceNotTriangulated,
        kMessageTypeFaceNullVertexObject,
        kMessageTypeFaceVertexObjectOutOfBound,
        kMessageTypeFaceVertexObjectNotUsed,
        kMessageTypeMaterialEmptyName,
        kMessageTypeMaterialDuplicatedName,
        kMessageTypeMaterialAmbientColorOutOfBound,
        kMessageTypeMaterialDiffuseColorOutOfBound,
        kMessageTypeMaterialDiffuseOpacityOutOfBound,
        kMessageTypeMaterialSpecularColorOutOfBound,
        kMessageTypeMaterialEdgeColorOutOfBound,
        kMessageTypeMaterialEdgeOpacityOutOfBound,
        kMessageTypeMaterialDiffuseTextureNotFound,
        kMessageTypeMaterialSphereMapTextureNotFound,
        kMessageTypeMaterialToonTextureNotFound,
        kMessageTypeMaterialVertexIndexNotFill,
        kMessageTypeMaterialVertexIndexOverflow,
        kMessageTypeBoneTooLongName,
        kMessageTypeBoneEmptyName,
        kMessageTypeBoneDuplicatedName,
        kMessageTypeBoneTransformBeforeParent,
        kMessageTypeBoneInherentBoneNullBoneObject,
        kMessageTypeBoneTransformBeforeInherentParent,
        kMessageTypeBoneTransformBeforeConstraint,
        kMessageTypeBoneFixedAxisNotNormalized,
        kMessageTypeMorphTooLongName,
        kMessageTypeMorphEmptyName,
        kMessageTypeMorphDuplicatedName,
        kMessageTypeLabelEmptyName,
        kMessageTypeLabelDuplicatedName,
        kMessageTypeLabelEmptyItems,
        kMessageTypeLabelNotAssignedBoneObject,
        kMessageTypeLabelNotAssignedMorphObject,
        kMessageTypeLabelItemNullBoneObject,
        kMessageTypeLabelItemNullMorphObject,
        kMessageTypeRigidBodyEmptyName,
        kMessageTypeRigidBodyDuplicatedName,
        kMessageTypeRigidBodyNullBoneObject,
        kMessageTypeJointEmptyName,
        kMessageTypeJointDuplicatedName,
        kMessageTypeJointRigidBodyANullObject,
        kMessageTypeJointRigidBodyBNullObject,
        kMessageTypeSoftBodyEmptyName,
        kMessageTypeSoftBodyDuplicatedName,
        kMessageTypeSoftBodyNullMaterialObject,
        kMessageTypeMaxEnum,
    };
    enum SeverityType {
        kSeverityTypeFirstEnum = 1,
        kSeverityTypeInfo = 1 << 1,
        kSeverityTypeWarning = 1 << 2,
        kSeverityTypeError = 1 << 3,
        kSeverityTypeFatal = 1 << 4,
        kSeverityTypeMaxEnum = 1 << 5,
    };
    struct Diagnostics {
        MessageType m_message;
        SeverityType m_severity;
        union {
            const nanoem_model_vertex_t *m_vertexPtr;
            const nanoem_model_material_t *m_materialPtr;
            const nanoem_model_bone_t *m_bonePtr;
            const nanoem_model_constraint_t *m_constraintPtr;
            const nanoem_model_constraint_joint_t *m_constraintJointPtr;
            const nanoem_model_morph_t *m_morphPtr;
            const nanoem_model_label_t *m_labelPtr;
            const nanoem_model_label_item_t *m_labelItemPtr;
            const nanoem_model_rigid_body_t *m_rigidBodyPtr;
            const nanoem_model_joint_t *m_jointPtr;
            const nanoem_model_soft_body_t *m_softBodyPtr;
        } u;
    };
    typedef tinystl::vector<Diagnostics, TinySTLAllocator> DiagnosticsList;

    Validator();
    ~Validator() NANOEM_DECL_NOEXCEPT;

    void validate(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void format(const Diagnostics &diag, const ITranslator *translator, String &text);

private:
    void validateAllVertexObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllFaces(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllMaterialObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllBoneObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllMorphObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllLabelObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllRigidBodyObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllJointObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    void validateAllSoftBodyObjects(const Model *model, nanoem_u32_t filter, DiagnosticsList &result);
    bool validateParentBone(
        const nanoem_model_bone_t *bonePtr, MessageType type, nanoem_u32_t filter, Diagnostics *diag);
    bool validateParentBone(const nanoem_model_bone_t *bonePtr, const nanoem_model_bone_t *parentBonePtr,
        MessageType type, nanoem_u32_t filter, Diagnostics *diag);
    bool validateNameInShiftJIS(const nanoem_unicode_string_t *value, nanoem_rsize_t expected, MessageType type,
        nanoem_u32_t filter, nanoem_unicode_string_factory_t *factory, Diagnostics *diag);
    bool validateVector3(const nanoem_f32_t *value, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool validateNormal(
        const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool validateTexCoord(
        const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool validateEulerAngles(
        const nanoem_f32_t *value, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool validateColor(
        const nanoem_f32_t *value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool validateOpacity(
        const nanoem_f32_t value, MessageType type, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
    bool testDiagnosticsSeverity(
        SeverityType severity, nanoem_u32_t filter, Diagnostics *diag) const NANOEM_DECL_NOEXCEPT;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_VALIDATOR_H_ */
