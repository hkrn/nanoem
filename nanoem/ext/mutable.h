/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_MUTABLE_H_
#define NANOEM_EXT_MUTABLE_H_

#include "../nanoem.h"

/**
 * \defgroup nanoem nanoem
 * @{
 */

/**
 * \defgroup nanoem_mutable_buffer Mutable Buffer
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_mutable_buffer_t);

/**
 * \brief 
 * 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_buffer_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreate(nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param capacity 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_buffer_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreateWithReservedSize(nanoem_rsize_t capacity, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteByte(nanoem_mutable_buffer_t *buffer, nanoem_u8_t value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt16LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i16_t value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt16LittleEndianUnsigned(nanoem_mutable_buffer_t *buffer, nanoem_u16_t value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i32_t value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteFloat32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_f32_t value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param data 
 * \param len 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteByteArray(nanoem_mutable_buffer_t *buffer, const nanoem_u8_t *data, nanoem_rsize_t len, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 * \param status 
 * \return NANOEM_DECL_API nanoem_buffer_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_buffer_t *APIENTRY
nanoemMutableBufferCreateBufferObject(nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param buffer 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferDestroy(nanoem_mutable_buffer_t *buffer);
/** @} */

/**
 * \defgroup nanoem_mutable_motion Mutable Motion
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_effect_parameter_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_outside_parent_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_accessory_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_bone_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_camera_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_light_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_model_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_model_keyframe_constraint_state_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_morph_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_motion_self_shadow_keyframe_t);

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_mutable_motion_keyframe_type_t){
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_UNKNOWN = -1,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE = 0x1,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA = 0x2,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT = 0x4,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL = 0x8,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH = 0x10,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW = 0x20,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY = 0x40,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL = 0x7f,
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MAX_ENUM = 0x80
};

/**
 * \defgroup nanoem_mutable_motion_effect_parameter Mutable Effect Parameter
 * @{
 */

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param value 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateAsReference(nanoem_motion_effect_parameter_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param parameter 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetName(nanoem_mutable_motion_effect_parameter_t *parameter, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param parameter 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetType(nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_motion_effect_parameter_type_t value);

/**
 * \brief 
 * 
 * \param parameter 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetValue(nanoem_mutable_motion_effect_parameter_t *parameter, const void *value);

/**
 * \brief 
 * 
 * \param new_parameter 
 * \param parameter 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterCopy(nanoem_mutable_motion_effect_parameter_t *new_parameter, const nanoem_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param parameter 
 * \return NANOEM_DECL_API nanoem_motion_effect_parameter_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterGetOriginObject(nanoem_mutable_motion_effect_parameter_t *parameter);

/**
 * \brief 
 * 
 * \param parameter 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterDestroy(nanoem_mutable_motion_effect_parameter_t *parameter);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_effect_parameter Mutable Effect Parameter
 * @{
 */

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param origin 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateAsReference(nanoem_motion_outside_parent_t *origin, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param op 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetTargetBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param op 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetTargetObjectName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param op 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetSubjectBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param new_op 
 * \param op 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentCopy(nanoem_mutable_motion_outside_parent_t *new_op, const nanoem_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param op 
 * \return NANOEM_DECL_API nanoem_motion_outside_parent_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentGetOriginObject(nanoem_mutable_motion_outside_parent_t *op);

/**
 * \brief 
 * 
 * \param op 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentDestroy(nanoem_mutable_motion_outside_parent_t *op);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_accessory_keyframe Mutable Accessory Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param origin 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateAsReference(nanoem_motion_accessory_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetTranslation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOrientation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetScaleFactor(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOpacity(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetVisible(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOutsideParent(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeCopy(nanoem_mutable_motion_accessory_keyframe_t *new_keyframe, const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param parameter 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeAddEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param parameter 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_accessory_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeGetOriginObject(nanoem_mutable_motion_accessory_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeDestroy(nanoem_mutable_motion_accessory_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_bone_keyframe Mutable Bone Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param origin 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateAsReference(nanoem_motion_bone_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param name 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetTranslation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetOrientation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param index 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetInterpolation(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index, const nanoem_u8_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetStageIndex(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_u32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeCopy(nanoem_mutable_motion_bone_keyframe_t *new_keyframe, const nanoem_motion_bone_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_bone_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeGetOriginObject(nanoem_mutable_motion_bone_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSaveToBuffer(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeDestroy(nanoem_mutable_motion_bone_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_camera_keyframe Mutable Camera Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param origin 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateAsReference(nanoem_motion_camera_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetOutsideParent(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetLookAt(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetAngle(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetDistance(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetFov(nanoem_mutable_motion_camera_keyframe_t *keyframe, int value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetPerspectiveView(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param index 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetInterpolation(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index, const nanoem_u8_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetStageIndex(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_u32_t value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeCopy(nanoem_mutable_motion_camera_keyframe_t *new_keyframe, const nanoem_motion_camera_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_camera_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeGetOriginObject(nanoem_mutable_motion_camera_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSaveToBuffer(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeDestroy(nanoem_mutable_motion_camera_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_light_keyframe Mutable Light Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateAsReference(nanoem_motion_light_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSetColor(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSetDirection(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeCopy(nanoem_mutable_motion_light_keyframe_t *new_keyframe, const nanoem_motion_light_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_light_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeGetOriginObject(nanoem_mutable_motion_light_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSaveToBuffer(nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeDestroy(nanoem_mutable_motion_light_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_model_keyframe Mutable Model Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateAsReference(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetVisible(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeColor(nanoem_mutable_motion_model_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetAddBlendEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeCopy(nanoem_mutable_motion_model_keyframe_t *new_keyframe, const nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_model_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeGetOriginObject(nanoem_mutable_motion_model_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param state 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param state 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param op 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param op 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param parameter 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param parameter 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSaveToBuffer(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeDestroy(nanoem_mutable_motion_model_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_model_keyframe Mutable Model Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreate(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param origin 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateAsReference(nanoem_motion_model_keyframe_constraint_state_t *origin, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param state 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param state 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param state 
 * \return NANOEM_DECL_API nanoem_motion_model_keyframe_constraint_state_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateGetOriginObject(nanoem_mutable_motion_model_keyframe_constraint_state_t *state);

/**
 * \brief 
 * 
 * \param state 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSaveToBuffer(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param state 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateDestroy(nanoem_mutable_motion_model_keyframe_constraint_state_t *state);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_morph_keyframe Mutable Morph Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateAsReference(nanoem_motion_morph_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param name 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeSetWeight(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeCopy(nanoem_mutable_motion_morph_keyframe_t *new_keyframe, const nanoem_motion_morph_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_morph_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeGetOriginObject(nanoem_mutable_motion_morph_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeSaveToBuffer(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeDestroy(nanoem_mutable_motion_morph_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_self_shadow_keyframe Mutable Self Shadow Keyframe
 * @{
 */

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateAsReference(nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param index 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetDistance(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetMode(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, int value);

/**
 * \brief 
 * 
 * \param new_keyframe 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeCopy(nanoem_mutable_motion_self_shadow_keyframe_t *new_keyframe, const nanoem_motion_self_shadow_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \return NANOEM_DECL_API nanoem_motion_self_shadow_keyframe_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeGetOriginObject(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe);

/**
 * \brief 
 * 
 * \param keyframe 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param keyframe 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeDestroy(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe);
/** @} */

/**
 * \brief 
 * 
 * \param factory 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_motion_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreateAsReference(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param name 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param name 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param frame_index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param keyframe 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSortAllKeyframes(nanoem_mutable_motion_t *motion);

/**
 * \brief 
 * 
 * \param motion 
 * \param key 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSetAnnotation(nanoem_mutable_motion_t *motion, const char *key, const char *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSetTargetModelName(nanoem_mutable_motion_t *motion, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 * \return NANOEM_DECL_API nanoem_motion_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObject(nanoem_mutable_motion_t *motion);

/**
 * \brief 
 * 
 * \param motion 
 * \return NANOEM_DECL_API nanoem_motion_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObjectReference(nanoem_mutable_motion_t *motion);

/**
 * \brief 
 * 
 * \param motion 
 * \param buffer 
 * \param status 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBuffer(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param motion 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionResetAllocationSize(nanoem_mutable_motion_t *motion);

/**
 * \brief 
 * 
 * \param motion 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionDestroy(nanoem_mutable_motion_t *motion);
/** @} */

/**
 * \defgroup nanoem_mutable_model Mutable Model
 * @{
 */
NANOEM_DECL_OPAQUE(nanoem_mutable_model_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_vertex_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_material_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_bone_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_constraint_joint_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_constraint_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_group_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_vertex_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_bone_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_uv_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_material_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_flip_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_impulse_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_morph_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_label_item_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_label_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_rigid_body_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_joint_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_soft_body_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_soft_body_anchor_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_model_texture_t);

/**
 * \defgroup nanoem_mutable_model_vertex Mutable Vertex
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param vertex 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreateAsReference(nanoem_model_vertex_t *vertex, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetOrigin(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetNormal(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetTexCoord(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 * \param index 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetAdditionalUV(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value, nanoem_rsize_t index);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefC(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefR0(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefR1(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 * \param index 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetBoneObject(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_bone_t *value, nanoem_rsize_t index);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 * \param index 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetBoneWeight(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value, nanoem_rsize_t index);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetEdgeSize(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetType(nanoem_mutable_model_vertex_t *vertex, nanoem_model_vertex_type_t value);

/**
 * \brief 
 * 
 * \param vertex 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexCopy(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_vertex_t *value);

/**
 * \brief 
 * 
 * \param vertex 
 * \return NANOEM_DECL_API nanoem_model_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_vertex_t *APIENTRY
nanoemMutableModelVertexGetOriginObject(nanoem_mutable_model_vertex_t *vertex);

/**
 * \brief 
 * 
 * \param vertex 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSaveToBuffer(nanoem_mutable_model_vertex_t *vertex, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param vertex 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexDestroy(nanoem_mutable_model_vertex_t *vertex);
/** @} */

/**
 * \defgroup nanoem_mutable_model_material Mutable Material
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreateAsReference(nanoem_model_material_t *material, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetName(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetClob(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetAmbientColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSpecularColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeSize(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSpecularPower(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureType(nanoem_mutable_model_material_t *material, nanoem_model_material_sphere_map_texture_type_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetNumVertexIndices(nanoem_mutable_model_material_t *material, nanoem_rsize_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonTextureIndex(nanoem_mutable_model_material_t *material, int value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonShared(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCullingDisabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCastingShadowEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCastingShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetVertexColorEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetPointDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetLineDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param material 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialCopy(nanoem_mutable_model_material_t *material, const nanoem_model_material_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 * \return NANOEM_DECL_API nanoem_model_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_material_t *APIENTRY
nanoemMutableModelMaterialGetOriginObject(nanoem_mutable_model_material_t *material);

/**
 * \brief 
 * 
 * \param material 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSaveToBuffer(nanoem_mutable_model_material_t *material, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param material 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialDestroy(nanoem_mutable_model_material_t *material);
/** @} */

/**
 * \defgroup nanoem_mutable_model_bone Mutable Bone
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param bone 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreateAsReference(nanoem_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetName(nanoem_mutable_model_bone_t *bone, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetEffectorBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetTargetBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneRemoveConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetDestinationOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetFixedAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalXAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalZAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentCoefficient(nanoem_mutable_model_bone_t *bone, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetStageIndex(nanoem_mutable_model_bone_t *bone, int value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetOffsetRelative(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetRotateable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetMovable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetVisible(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetUserHandleable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetConstraintEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalInherentEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentTranslationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentOrientationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetFixedAxisEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalAxesEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetAffectedByPhysicsSimulation(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneEnableExternalParentBone(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param bone 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneCopy(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param bone 
 * \return NANOEM_DECL_API nanoem_model_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_bone_t *APIENTRY
nanoemMutableModelBoneGetOriginObject(nanoem_mutable_model_bone_t *bone);

/**
 * \brief 
 * 
 * \param bone 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSaveToBuffer(nanoem_mutable_model_bone_t *bone, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param bone 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneDestroy(nanoem_mutable_model_bone_t *bone);
/** @} */

/**
 * \defgroup nanoem_mutable_model_constraint Mutable Constraint
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_constraint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_constraint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreateAsReference(nanoem_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetEffectorBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetTargetBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetAngleLimit(nanoem_mutable_model_constraint_t *constraint, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetNumIterations(nanoem_mutable_model_constraint_t *constraint, int value);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintInsertJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintRemoveJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintCopy(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_constraint_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 * \return NANOEM_DECL_API nanoem_model_constraint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_constraint_t *APIENTRY
nanoemMutableModelConstraintGetOriginObject(nanoem_mutable_model_constraint_t *constraint);

/**
 * \brief 
 * 
 * \param constraint 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSaveToBuffer(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param constraint 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintDestroy(nanoem_mutable_model_constraint_t *constraint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_constraint_joint Mutable Constraint Joint
 * @{
 */

/**
 * \brief 
 * 
 * \param constraint 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreate(nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreateAsReference(nanoem_model_constraint_joint_t *joint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetBoneObject(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetUpperLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetLowerLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetAngleLimitEnabled(nanoem_mutable_model_constraint_joint_t *joint, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointCopy(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_constraint_joint_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \return NANOEM_DECL_API nanoem_model_constraint_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointGetOriginObject(nanoem_mutable_model_constraint_joint_t *joint);

/**
 * \brief 
 * 
 * \param joint 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSaveToBuffer(nanoem_mutable_model_constraint_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointDestroy(nanoem_mutable_model_constraint_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_texture Mutable Texture
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_texture_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param texture 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_texture_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreateAsReference(nanoem_model_texture_t *texture, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param texture 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureSetPath(nanoem_mutable_model_texture_t *texture, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param texture 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureCopy(nanoem_mutable_model_texture_t *texture, const nanoem_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param texture 
 * \return NANOEM_DECL_API nanoem_model_texture_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_texture_t *APIENTRY
nanoemMutableModelTextureGetOriginObject(nanoem_mutable_model_texture_t *texture);

/**
 * \brief 
 * 
 * \param texture 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureSaveToBuffer(nanoem_mutable_model_texture_t *texture, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param texture 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureDestroy(nanoem_mutable_model_texture_t *texture);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph Mutable Morph
 * @{
 */

/**
 * \defgroup nanoem_mutable_model_morph_bone Mutable Bone Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreateAsReference(nanoem_model_morph_bone_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetBoneObject(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetTranslation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetOrientation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneCopy(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_morph_bone_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSaveToBuffer(nanoem_mutable_model_morph_bone_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_bone_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneGetOriginObject(nanoem_mutable_model_morph_bone_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneDestroy(nanoem_mutable_model_morph_bone_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_flip Mutable Flip Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_flip_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_flip_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreateAsReference(nanoem_model_morph_flip_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSetMorphObject(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSetWeight(nanoem_mutable_model_morph_flip_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipCopy(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_flip_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSaveToBuffer(nanoem_mutable_model_morph_flip_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_flip_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipGetOriginObject(nanoem_mutable_model_morph_flip_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipDestroy(nanoem_mutable_model_morph_flip_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_group Mutable Group Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_group_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_group_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreateAsReference(nanoem_model_morph_group_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSetMorphObject(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSetWeight(nanoem_mutable_model_morph_group_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupCopy(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_group_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSaveToBuffer(nanoem_mutable_model_morph_group_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_group_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupGetOriginObject(nanoem_mutable_model_morph_group_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupDestroy(nanoem_mutable_model_morph_group_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_impulse Mutable Impulse Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreateAsReference(nanoem_model_morph_impulse_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetRigidBodyObject(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_rigid_body_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetTorque(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetVelocity(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetLocal(nanoem_mutable_model_morph_impulse_t *morph, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseCopy(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_morph_impulse_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSaveToBuffer(nanoem_mutable_model_morph_impulse_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_impulse_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseGetOriginObject(nanoem_mutable_model_morph_impulse_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseDestroy(nanoem_mutable_model_morph_impulse_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_material Mutable Material Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreateAsReference(nanoem_model_morph_material_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetMaterialObject(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_material_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetAmbientColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetToonTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularPower(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeSize(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetOperationType(nanoem_mutable_model_morph_material_t *morph, nanoem_model_morph_material_operation_type_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialCopy(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_morph_material_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSaveToBuffer(nanoem_mutable_model_morph_material_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_material_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialGetOriginObject(nanoem_mutable_model_morph_material_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialDestroy(nanoem_mutable_model_morph_material_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_uv Mutable UV Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_uv_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_uv_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreateAsReference(nanoem_model_morph_uv_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSetVertexObject(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_vertex_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSetPosition(nanoem_mutable_model_morph_uv_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVCopy(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_morph_uv_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSaveToBuffer(nanoem_mutable_model_morph_uv_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_uv_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVGetOriginObject(nanoem_mutable_model_morph_uv_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVDestroy(nanoem_mutable_model_morph_uv_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_vertex Mutable Vertex Morph
 * @{
 */

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreateAsReference(nanoem_model_morph_vertex_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSetVertexObject(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_vertex_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSetPosition(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexCopy(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_morph_vertex_t *value);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSaveToBuffer(nanoem_mutable_model_morph_vertex_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_vertex_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexGetOriginObject(nanoem_mutable_model_morph_vertex_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexDestroy(nanoem_mutable_model_morph_vertex_t *morph);
/** @} */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_morph_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreateAsReference(nanoem_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetName(nanoem_mutable_model_morph_t *morph, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetCategory(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_category_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetType(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_type_t value);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphCopy(nanoem_mutable_model_morph_t *morph, const nanoem_model_morph_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 * \return NANOEM_DECL_API nanoem_model_morph_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_morph_t *APIENTRY
nanoemMutableModelMorphGetOriginObject(nanoem_mutable_model_morph_t *morph);

/**
 * \brief 
 * 
 * \param morph 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSaveToBuffer(nanoem_mutable_model_morph_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param morph 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphDestroy(nanoem_mutable_model_morph_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_label Mutable Label
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_label_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_label_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreateAsReference(nanoem_model_label_t *label, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSetName(nanoem_mutable_model_label_t *label, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSetSpecial(nanoem_mutable_model_label_t *label, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param label 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelCopy(nanoem_mutable_model_label_t *label, const nanoem_model_label_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \return NANOEM_DECL_API nanoem_model_label_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_label_t *APIENTRY
nanoemMutableModelLabelGetOriginObject(nanoem_mutable_model_label_t *label);

/**
 * \brief 
 * 
 * \param label 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSaveToBuffer(nanoem_mutable_model_label_t *label, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelDestroy(nanoem_mutable_model_label_t *label);

/**
 * \defgroup nanoem_mutable_model_label_item Mutable Label Item
 * @{
 */

/**
 * \brief 
 * 
 * \param item 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_label_item_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateAsReference(nanoem_model_label_item_t *item, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param bone 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_label_item_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromBoneObject(nanoem_mutable_model_label_t *label, nanoem_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param morph 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_label_item_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromMorphObject(nanoem_mutable_model_label_t *label, nanoem_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param item 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelInsertItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \param item 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelRemoveItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param label 
 * \return NANOEM_DECL_API nanoem_model_label_item_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemGetOriginObject(nanoem_mutable_model_label_item_t *label);

/**
 * \brief 
 * 
 * \param item 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelItemDestroy(nanoem_mutable_model_label_item_t *item);
/** @} */

/** @} */

/**
 * \defgroup nanoem_mutable_model_rigid_body Mutable Rigid Body
 * @{
 */
NANOEM_DECL_API nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_rigid_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreateAsReference(nanoem_model_rigid_body_t *rigid_body, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetName(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetBoneObject(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_bone_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetOrigin(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetOrientation(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetShapeSize(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetMass(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetLinearDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetAngularDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetFriction(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetRestitution(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetShapeType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_shape_type_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetTransformType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_transform_type_t value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetCollisionGroupId(nanoem_mutable_model_rigid_body_t *rigid_body, int value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetCollisionMask(nanoem_mutable_model_rigid_body_t *rigid_body, int value);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodyCopy(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_rigid_body_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \return NANOEM_DECL_API nanoem_model_rigid_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyGetOriginObject(nanoem_mutable_model_rigid_body_t *rigid_body);

/**
 * \brief 
 * 
 * \param rigid_body 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySaveToBuffer(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param rigid_body 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodyDestroy(nanoem_mutable_model_rigid_body_t *rigid_body);
/** @} */

/**
 * \defgroup nanoem_mutable_model_joint Mutable Joint
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreateAsReference(nanoem_model_joint_t *joint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetName(nanoem_mutable_model_joint_t *joint, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetRigidBodyAObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetRigidBodyBObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetOrigin(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetOrientation(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetType(nanoem_mutable_model_joint_t *joint, nanoem_model_joint_type_t value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief 
 * 
 * \param joint 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointCopy(nanoem_mutable_model_joint_t *joint, const nanoem_model_joint_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 * \return NANOEM_DECL_API nanoem_model_joint_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_joint_t *APIENTRY
nanoemMutableModelJointGetOriginObject(nanoem_mutable_model_joint_t *joint);

/**
 * \brief 
 * 
 * \param joint 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSaveToBuffer(nanoem_mutable_model_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param joint 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointDestroy(nanoem_mutable_model_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_softbody Mutable Soft Body
 * @{
 */

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_soft_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_soft_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreateAsReference(nanoem_model_soft_body_t *body, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetName(nanoem_mutable_model_soft_body_t *body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetMaterialObject(nanoem_mutable_model_soft_body_t *body, const nanoem_model_material_t *value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 * \param length 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAllPinnedVertexIndices(nanoem_mutable_model_soft_body_t *body, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetShapeType(nanoem_mutable_model_soft_body_t *body, nanoem_model_soft_body_shape_type_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAeroModel(nanoem_mutable_model_soft_body_t *body, nanoem_model_soft_body_aero_model_type_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetTotalMass(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionMargin(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVelocityCorrectionFactor(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDampingCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDragCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetLiftCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPressureCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVolumeConversationCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPoseMatchingCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetRigidContactHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetKineticContactHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftContactHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAnchorHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftHardness(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(nanoem_mutable_model_soft_body_t *body, nanoem_f32_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionGroupId(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionMask(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsDistance(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClusterCount(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVelocitySolverIterations(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPositionsSolverIterations(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDriftSolverIterations(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClusterSolverIterations(nanoem_mutable_model_soft_body_t *body, int value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsEnabled(nanoem_mutable_model_soft_body_t *body, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClustersEnabled(nanoem_mutable_model_soft_body_t *body, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(nanoem_mutable_model_soft_body_t *body, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param body 
 * \param anchor 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyInsertAnchorObject(nanoem_mutable_model_soft_body_t *body, nanoem_mutable_model_soft_body_anchor_t *anchor, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param anchor 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyRemoveAnchorObject(nanoem_mutable_model_soft_body_t *body, nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \param value 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyCopy(nanoem_mutable_model_soft_body_t *body, const nanoem_model_soft_body_t *value, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 * \return NANOEM_DECL_API nanoem_model_soft_body_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyGetOriginObject(nanoem_mutable_model_soft_body_t *body);

/**
 * \brief 
 * 
 * \param body 
 * \param buffer 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySaveToBuffer(nanoem_mutable_model_soft_body_t *body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param body 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyDestroy(nanoem_mutable_model_soft_body_t *body);

/**
 * \defgroup nanoem_mutable_model_softbody_anchor Mutable Soft Body Anchor
 * @{
 */

/**
 * \brief 
 * 
 * \param body 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreate(nanoem_mutable_model_soft_body_t *body, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param anchor 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreateAsReference(nanoem_model_soft_body_anchor_t *anchor, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param anchor 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetRigidBodyObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_rigid_body_t *value);

/**
 * \brief 
 * 
 * \param anchor 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetVertexObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_vertex_t *value);

/**
 * \brief 
 * 
 * \param anchor 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetNearEnabled(nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_bool_t value);

/**
 * \brief 
 * 
 * \param anchor 
 * \return NANOEM_DECL_API nanoem_model_soft_body_anchor_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorGetOriginObject(nanoem_mutable_model_soft_body_anchor_t *anchor);

/**
 * \brief 
 * 
 * \param anchor 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorDestroy(nanoem_mutable_model_soft_body_anchor_t *anchor);
/** @} */

/** @} */

/**
 * \brief 
 * 
 * \param factory 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param status 
 * \return NANOEM_DECL_API nanoem_mutable_model_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreateAsReference(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetFormatType(nanoem_mutable_model_t *model, nanoem_model_format_type_t value);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetCodecType(nanoem_mutable_model_t *model, nanoem_codec_type_t value);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetAdditionalUVSize(nanoem_mutable_model_t *model, nanoem_rsize_t value);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetName(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 * \param language 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetComment(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param value 
 * \param length 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetVertexIndices(nanoem_mutable_model_t *model, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param vertex 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param material 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param bone 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param constraint 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param texture 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param morph 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param label 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param rigid_body 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param joint 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param soft_body 
 * \param index 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, int index, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param vertex 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param material 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param bone 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param constraint 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param texture 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param morph 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param label 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param rigid_body 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param joint 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \param soft_body 
 * \param status 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 * \return NANOEM_DECL_API nanoem_model_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObject(nanoem_mutable_model_t *model);

/**
 * \brief 
 * 
 * \param model 
 * \return NANOEM_DECL_API nanoem_model_t* APIENTRY 
 */
NANOEM_DECL_API nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObjectReference(nanoem_mutable_model_t *model);

/**
 * \brief 
 * 
 * \param model 
 * \param buffer 
 * \param status 
 * \return NANOEM_DECL_API nanoem_bool_t APIENTRY 
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableModelSaveToBuffer(nanoem_mutable_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief 
 * 
 * \param model 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelResetAllocationSize(nanoem_mutable_model_t *model);

/**
 * \brief 
 * 
 * \param model 
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelDestroy(nanoem_mutable_model_t *model);
/** @} */

#endif /* NANOEM_EXT_MUTABLE_H_ */
