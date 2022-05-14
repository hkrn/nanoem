/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/Error.h"

#include "emapp/BaseApplicationService.h"
#include "emapp/IEventPublisher.h"
#include "emapp/ITranslator.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/StringUtils.h"

namespace nanoem {

const char *
Error::convertStatusToMessage(nanoem_status_t status, const ITranslator *translator) NANOEM_DECL_NOEXCEPT
{
    switch (status) {
    case NANOEM_STATUS_UNKNOWN:
        return translator->translate("nanoem.status.UNKNOWN");
    case NANOEM_STATUS_SUCCESS:
        return translator->translate("nanoem.status.SUCCESS");
    case NANOEM_STATUS_ERROR_MALLOC_FAILED:
        return translator->translate("nanoem.status.ERROR_MALLOC_FAILED");
    case NANOEM_STATUS_ERROR_REALLOC_FAILED:
        return translator->translate("nanoem.status.ERROR_REALLOC_FAILED");
    case NANOEM_STATUS_ERROR_NULL_OBJECT:
        return translator->translate("nanoem.status.ERROR_NULL_OBJECT");
    case NANOEM_STATUS_ERROR_BUFFER_END:
        return translator->translate("nanoem.status.ERROR_BUFFER_END");
    case NANOEM_STATUS_ERROR_DECODE_UNICODE_STRING_FAILED:
        return translator->translate("nanoem.status.ERROR_DECODE_UNICODE_STRING_FAILED");
    case NANOEM_STATUS_ERROR_ENCODE_UNICODE_STRING_FAILED:
        return translator->translate("nanoem.status.ERROR_ENCODE_UNICODE_STRING_FAILED");
    case NANOEM_STATUS_ERROR_INVALID_SIGNATURE:
        return translator->translate("nanoem.status.ERROR_INVALID_SIGNATURE");
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_VERTEX_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_FACE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_FACE_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_MATERIAL_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_BONE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_BONE_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_TEXTURE_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_LABEL_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_LABEL_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_RIGID_BODY_CORRUPTED");
    case NANOEM_STATUS_ERROR_MODEL_JOINT_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MODEL_JOINT_CORRUPTED");
    case NANOEM_STATUS_ERROR_PMD_ENGLISH_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_PMD_ENGLISH_CORRUPTED");
    case NANOEM_STATUS_ERROR_PMX_INFO_CORRUPUTED:
        return translator->translate("nanoem.status.ERROR_PMX_INFO_CORRUPUTED");
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_BONE_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_CAMERA_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_LIGHT_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_MODEL_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_MORPH_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_MOTION_SELF_SHADOW_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_BONE_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_BONE_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_BONE_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_CAMERA_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_CAMERA_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_CAMERA_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_LIGHT_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_LIGHT_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_LIGHT_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_MODEL_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_MODEL_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_MODEL_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_MORPH_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_MORPH_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_MORPH_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_SELF_SHADOW_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_SELF_SHADOW_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MOTION_ACCESSORY_KEYFRAME_REFERENCE");
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MOTION_ACCESSORY_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MOTION_ACCESSORY_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_REFERENCE:
        return translator->translate("nanoem.status.ERROR_EFFECT_PARAMETER_REFERENCE");
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_EFFECT_PARAMETER_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_EFFECT_PARAMETER_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_EFFECT_PARAMETER_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_STATE_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_STATE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_BINDING_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_BINDING_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_BINDING_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_BINDING_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_BINDING_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_BINDING_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_VERTEX_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_VERTEX_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_VERTEX_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_VERTEX_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_MATERIAL_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_MATERIAL_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_MATERIAL_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MATERIAL_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_BONE_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_BONE_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_BONE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_BONE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_BONE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_BONE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_CONSTRAINT_JOINT_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_TEXTURE_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_TEXTURE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_TEXTURE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_TEXTURE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_TYPE_MISMATCH:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_TYPE_MISMATCH");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_BONE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_BONE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_FLIP_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_FLIP_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_GROUP_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_GROUP_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_IMPULSE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_MATERIAL_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_UV_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_UV_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_MORPH_VERTEX_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_MORPH_VERTEX_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_LABEL_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_LABEL_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_LABEL_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_LABEL_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_LABEL_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_LABEL_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_LABEL_ITEM_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_LABEL_ITEM_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_RIGID_BODY_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_RIGID_BODY_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_RIGID_BODY_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_RIGID_BODY_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_JOINT_REFERENCE:
        return translator->translate("nanoem.status.ERROR_MODEL_JOINT_REFERENCE");
    case NANOEM_STATUS_ERROR_MODEL_JOINT_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_MODEL_JOINT_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_MODEL_JOINT_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_MODEL_JOINT_NOT_FOUND");
    case NANOEM_STATUS_ERROR_MODEL_VERSION_INCOMPATIBLE:
        return translator->translate("nanoem.status.ERROR_MODEL_VERSION_INCOMPATIBLE");
    case NANOEM_STATUS_MAX_ENUM:
        return translator->translate("nanoem.status.MAX_ENUM");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_CAMERA_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_CAMERA_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_GRAVITY_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_GRAVITY_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_LIGHT_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_LIGHT_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_STATE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_STATE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MODEL_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_STATE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_STATE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_STATE_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_ALREADY_EXISTS");
    case NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_NOT_FOUND:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_NOT_FOUND");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_ACCESSORY_OUTSIDE_PARENT_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_ACCESSORY_OUTSIDE_PARENT_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_CAMERA_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_CAMERA_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_CAMERA_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_GRAVITY_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_GRAVITY_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_GRAVITY_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_LIGHT_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_LIGHT_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_LIGHT_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_BONE_STATE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_BONE_STATE_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_CONSTRAINT_STATE_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_KEYFRAME_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_MORPH_STATE_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_MORPH_STATE_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_MODEL_OUTSIDE_PARENT_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_SELF_SHADOW_CORRUPTED");
    case NANOEM_STATUS_ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_CORRUPTED:
        return translator->translate("nanoem.status.ERROR_DOCUMENT_SELF_SHADOW_KEYFRAME_CORRUPTED");
    default:
        return "(unknown)";
    }
}

const char *
Error::convertDomainToString(DomainType value) NANOEM_DECL_NOEXCEPT
{
    switch (value) {
    case kDomainTypeApplication:
        return "application";
    case kDomainTypeCancel:
        return "cancel";
    case kDomainTypeMinizip:
        return "minizip";
    case kDomainTypeNanodxm:
        return "nanodxm";
    case kDomainTypeNanoem:
        return "nanoem";
    case kDomainTypeNanomqo:
        return "nanomqo";
    case kDomainTypeOS:
        return "os";
    case kDomainTypePlugin:
        return "plugin";
    default:
        return "(unknown)";
    }
}

Error
Error::cancelled()
{
    return Error("Cancelleration request has been issued", 0, kDomainTypeCancel);
}

Error::Error() NANOEM_DECL_NOEXCEPT : m_domain(kDomainTypeUnknown), m_code(0)
{
    *m_reason = *m_recoverySuggestion = 0;
}

Error::Error(const char *reason, int code, DomainType domain) NANOEM_DECL_NOEXCEPT : m_domain(domain), m_code(code)
{
    *m_recoverySuggestion = 0;
    StringUtils::copyString(m_reason, reason, sizeof(m_reason));
}

Error::Error(const char *reason, const char *recoverySuggestion, DomainType domain) NANOEM_DECL_NOEXCEPT
    : m_domain(domain),
      m_code(0)
{
    StringUtils::copyString(m_reason, reason, sizeof(m_reason));
    if (recoverySuggestion) {
        StringUtils::copyString(m_recoverySuggestion, recoverySuggestion, sizeof(m_recoverySuggestion));
    }
}

Error::Error(const Error &value) NANOEM_DECL_NOEXCEPT
{
    this->operator=(value);
}

Error::~Error() NANOEM_DECL_NOEXCEPT
{
}

IModalDialog *
Error::createModalDialog(BaseApplicationService *applicationPtr) const
{
    IModalDialog *dialog = nullptr;
    if (hasReason() && !isCancelled()) {
        static const nanoem_u8_t kFAExclamationTriangle[] = { 0xef, 0x81, 0xb1, 0 };
        const ITranslator *translator = applicationPtr->translator();
        const String title(translator->translate("nanoem.window.dialog.error.title"));
        String buffer;
        buffer.append(reinterpret_cast<const char *>(kFAExclamationTriangle));
        buffer.append(" ");
        buffer.append(translator->translate("nanoem.window.dialog.error.message.reason"));
        buffer.append("\n");
        int c = code();
        if (c != 0) {
            char partial[16];
            StringUtils::format(partial, sizeof(partial), "(%d): ", c);
            buffer.append(partial);
        }
        buffer.append(reasonConstString());
        if (hasRecoverySuggestion()) {
            static const nanoem_u8_t kFAInfoCircle[] = { 0xef, 0x81, 0x9a, 0 };
            buffer.append("\n\n");
            buffer.append(reinterpret_cast<const char *>(kFAInfoCircle));
            buffer.append(" ");
            buffer.append(translator->translate("nanoem.window.dialog.error.message.recovery-suggestion"));
            buffer.append("\n");
            buffer.append(recoverySuggestionConstString());
        }
        dialog = ModalDialogFactory::createDisplayPlainTextDialog(applicationPtr, title, buffer);
    }
    return dialog;
}

void
Error::addModalDialog(BaseApplicationService *applicationPtr) const
{
    if (IModalDialog *dialog = createModalDialog(applicationPtr)) {
        applicationPtr->addModalDialog(dialog);
        applicationPtr->eventPublisher()->publishErrorEvent(*this);
    }
}

void
Error::notify(IEventPublisher *publisher) const
{
    if (hasReason()) {
        publisher->publishErrorEvent(*this);
    }
}

bool
Error::hasReason() const NANOEM_DECL_NOEXCEPT
{
    return m_reason[0] != 0;
}

bool
Error::hasRecoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    return m_recoverySuggestion[0] != 0;
}

const char *
Error::reasonConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_reason;
}

const char *
Error::recoverySuggestionConstString() const NANOEM_DECL_NOEXCEPT
{
    return m_recoverySuggestion;
}

Error::DomainType
Error::domain() const NANOEM_DECL_NOEXCEPT
{
    return m_domain;
}

int
Error::code() const NANOEM_DECL_NOEXCEPT
{
    return m_code;
}

bool
Error::isCancelled() const NANOEM_DECL_NOEXCEPT
{
    return m_domain == kDomainTypeCancel;
}

void
Error::operator=(const Error &value) NANOEM_DECL_NOEXCEPT
{
    m_domain = value.m_domain;
    m_code = value.m_code;
    StringUtils::copyString(m_reason, value.m_reason, sizeof(m_reason));
    StringUtils::copyString(m_recoverySuggestion, value.m_recoverySuggestion, sizeof(m_recoverySuggestion));
}

} /* namespace nanoem */
