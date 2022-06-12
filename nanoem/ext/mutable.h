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
 * \brief Create an opaque mutable buffer object
 *
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreate(nanoem_status_t *status);

/**
 * \brief Create an opaque mutable buffer object with reserved capacity
 *
 * \param capacity size to pre-allocate
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_buffer_t *APIENTRY
nanoemMutableBufferCreateWithReservedSize(nanoem_rsize_t capacity, nanoem_status_t *status);

/**
 * \brief Write an unsigned 8bits integer to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteByte(nanoem_mutable_buffer_t *buffer, nanoem_u8_t value, nanoem_status_t *status);

/**
 * \brief Write a signed 16bits integer to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt16LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i16_t value, nanoem_status_t *status);

/**
 * \brief Write an unsigned 16bits integer to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt16LittleEndianUnsigned(nanoem_mutable_buffer_t *buffer, nanoem_u16_t value, nanoem_status_t *status);

/**
 * \brief Write a signed 32bits integer to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteInt32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_i32_t value, nanoem_status_t *status);

/**
 * \brief Write an unsigned 32bits integer to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteFloat32LittleEndian(nanoem_mutable_buffer_t *buffer, nanoem_f32_t value, nanoem_status_t *status);

/**
 * \brief Write byte array to the given opaque buffer object
 *
 * \param buffer The opaque buffer object
 * \param data The byte array data to be written
 * \param len size to write of \b data in byte unit
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableBufferWriteByteArray(nanoem_mutable_buffer_t *buffer, const nanoem_u8_t *data, nanoem_rsize_t len, nanoem_status_t *status);

/**
 * \brief Create an opaque immutable buffer object from the given opaque mutable buffer object
 *
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_buffer_t *APIENTRY
nanoemMutableBufferCreateBufferObject(nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque buffer object
 *
 * \param buffer The opaque buffer object
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
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_UNKNOWN = -1, ///< Unknown
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_BONE = 0x1, ///< Bone
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA = 0x2, ///< Camera
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT = 0x4, ///< Light
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL = 0x8, ///< Model
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MORPH = 0x10, ///< Morph
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW = 0x20, ///< Self Shadow
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY = 0x40, ///< Accessory
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ALL = 0x7f, ///< All
    NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MAX_ENUM = 0x80
};

/**
 * \defgroup nanoem_mutable_motion_effect_parameter Mutable Motion Effect Parameter
 * @{
 */

/**
 * \brief Create an opaque effect parameter object from the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque effect parameter object from the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque effect parameter object from the given existing object
 *
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterCreateAsReference(nanoem_motion_effect_parameter_t *value, nanoem_status_t *status);

/**
 * \brief Set the name to the given opaque effect parameter object
 *
 * \param parameter The opaque effect parameter object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetName(nanoem_mutable_motion_effect_parameter_t *parameter, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Set the type to the given opaque effect parameter object
 *
 * \param parameter The opaque effect parameter object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetType(nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_motion_effect_parameter_type_t value);

/**
 * \brief Set the opaque value to the given opaque effect parameter object
 *
 * \param parameter The opaque effect parameter object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterSetValue(nanoem_mutable_motion_effect_parameter_t *parameter, const void *value);

/**
 * \brief Copy contents of the given opaque effect parameter object
 *
 * All children of the opaque effect parameter object are cloned and added to the destination \b new_parameter
 *
 * \param new_parameter The destination opaque effect parameter object
 * \param parameter The source opaque effect parameter object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterCopy(nanoem_mutable_motion_effect_parameter_t *new_parameter, const nanoem_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief Get the opaque origin effect parameter object from the given opaque effect parameter object
 *
 * \param parameter The opaque effect parameter object
 */
NANOEM_DECL_API nanoem_motion_effect_parameter_t *APIENTRY
nanoemMutableMotionEffectParameterGetOriginObject(nanoem_mutable_motion_effect_parameter_t *parameter);

/**
 * \brief Destroy the given opaque effect parameter object
 *
 * \param parameter The opaque effect parameter object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionEffectParameterDestroy(nanoem_mutable_motion_effect_parameter_t *parameter);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_outside_parent Mutable Motion Outside Parent
 * @{
 */

/**
 * \brief Create an opaque motion outside parent object from the given motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque motion outside parent object from the given motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromCameraKeyframe(nanoem_motion_camera_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque motion outside parent object from the given motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateFromModelKeyframe(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque motion outside parent object from the given existing object
 *
 * \param origin the existing opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentCreateAsReference(nanoem_motion_outside_parent_t *origin, nanoem_status_t *status);

/**
 * \brief Set the target bone name to the given opaque outside parent object
 *
 * \param op The opaque outside parent object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetTargetBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Set the target object name to the given opaque outside parent object
 *
 * \param op The opaque outside parent object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetTargetObjectName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Set the subject bone name to the given opaque outside parent object
 *
 * \param op The opaque outside parent object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentSetSubjectBoneName(nanoem_mutable_motion_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque motion outside parent object
 *
 * All children of the opaque motion outside parent object are cloned and added to the destination \b new_op
 *
 * \param new_op The destination opaque outside parent object
 * \param op The source opaque outside parent object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentCopy(nanoem_mutable_motion_outside_parent_t *new_op, const nanoem_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief Get the opaque origin outside parent object from the given its object
 *
 * \param op The opaque outside parent object
 */
NANOEM_DECL_API nanoem_motion_outside_parent_t *APIENTRY
nanoemMutableMotionOutsideParentGetOriginObject(nanoem_mutable_motion_outside_parent_t *op);

/**
 * \brief Destroy the given opaque outside parent object
 *
 * \param op The opaque outside parent object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionOutsideParentDestroy(nanoem_mutable_motion_outside_parent_t *op);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_accessory_keyframe Mutable Accessory Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion accessory keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion accessory keyframe object from the given existing object
 *
 * \param origin the existing opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateAsReference(nanoem_motion_accessory_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief Find an opaque motion accessory keyframe object from the given frame index
 *
 * \param motion The opaque motion object
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionAccessoryKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the translation vector to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetTranslation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the orientation euler angles in radians to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOrientation(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the scale factor to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetScaleFactor(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Set the opacity to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOpacity(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Set whether add blend is enabled to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetAddBlendEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Set whether projection shadow is enabled to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetShadowEnabled(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Set whether the accessory is visible to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetVisible(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Set the opaque outside parent object to the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeSetOutsideParent(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque motion accessory keyframe object
 *
 * All children of the opaque motion accessory keyframe object are cloned and added to the destination \b new_keyframe
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeCopy(nanoem_mutable_motion_accessory_keyframe_t *new_keyframe, const nanoem_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Add the given opaque effect parameter object to the associated opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param parameter The opaque effect parameter object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeAddEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief Remove the given opaque effect parameter object from the associated opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param parameter The opaque effect parameter object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeRemoveEffectParameter(nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief Get the opaque origin motion accessory keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_accessory_keyframe_t *APIENTRY
nanoemMutableMotionAccessoryKeyframeGetOriginObject(nanoem_mutable_motion_accessory_keyframe_t *keyframe);

/**
 * \brief Destroy the given opaque motion accessory keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeDestroy(nanoem_mutable_motion_accessory_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_bone_keyframe Mutable Bone Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion bone keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion bone keyframe object from the given existing object
 *
 * \param origin the existing opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateAsReference(nanoem_motion_bone_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief Find an opaque motion bone keyframe object from the given name and frame index
 *
 * \param motion The opaque motion object
 * \param name The name to find
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionBoneKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the translation vector to the given opaque motion bone keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetTranslation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the orientation quaternion to the given opaque motion bone keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetOrientation(nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the interpolation vector to the given opaque motion bone keyframe object and the index
 *
 * \param keyframe The opaque motion keyframe object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetInterpolation(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t index, const nanoem_u8_t *value);

/**
 * \brief Set the stage index to the given opaque motion bone keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetStageIndex(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_u32_t value);

/**
 * \brief Set whether physics simulation is enabled to the given opaque motion bone keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque motion bone keyframe object
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeCopy(nanoem_mutable_motion_bone_keyframe_t *new_keyframe, const nanoem_motion_bone_keyframe_t *keyframe);

/**
 * \brief Get the opaque origin motion bone keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_bone_keyframe_t *APIENTRY
nanoemMutableMotionBoneKeyframeGetOriginObject(nanoem_mutable_motion_bone_keyframe_t *keyframe);

/**
 * \brief Serialize the opaque motion bone keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeSaveToBuffer(nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion bone keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeDestroy(nanoem_mutable_motion_bone_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_camera_keyframe Mutable Camera Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion camera keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion camera keyframe object from the given existing object
 *
 * \param origin the existing opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateAsReference(nanoem_motion_camera_keyframe_t *origin, nanoem_status_t *status);

/**
 * \brief Find an opaque motion camera keyframe object from the given frame index
 *
 * \param motion The opaque motion object
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionCameraKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the opaque outside parent object to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetOutsideParent(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *value, nanoem_status_t *status);

/**
 * \brief Set the lookat vector to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetLookAt(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the angle vector in radians to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetAngle(nanoem_mutable_motion_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the distance to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetDistance(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Set the fov in degrees to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetFov(nanoem_mutable_motion_camera_keyframe_t *keyframe, int value);

/**
 * \brief Set whether it's perspective view to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetPerspectiveView(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Get the interpolation vector from the given opaque motion camera keyframe object and the index
 *
 * \param keyframe The opaque motion keyframe object
 * \param index The index to set
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetInterpolation(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t index, const nanoem_u8_t *value);

/**
 * \brief Set the stage index to the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSetStageIndex(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_u32_t value);

/**
 * \brief Copy contents of the given opaque motion camera keyframe object
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeCopy(nanoem_mutable_motion_camera_keyframe_t *new_keyframe, const nanoem_motion_camera_keyframe_t *keyframe);

/**
 * \brief Get the opaque origin motion camera keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_camera_keyframe_t *APIENTRY
nanoemMutableMotionCameraKeyframeGetOriginObject(nanoem_mutable_motion_camera_keyframe_t *keyframe);

/**
 * \brief Serialize the opaque motion camera keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeSaveToBuffer(nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion camera keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeDestroy(nanoem_mutable_motion_camera_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_light_keyframe Mutable Light Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion light keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion light keyframe object from the given existing object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateAsReference(nanoem_motion_light_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Find an opaque motion light keyframe object from the given frame index
 *
 * \param motion The opaque motion object
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionLightKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the color to the given opaque motion light keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSetColor(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set the direction to the given opaque motion light keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSetDirection(nanoem_mutable_motion_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Copy contents of the given opaque motion light keyframe object
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeCopy(nanoem_mutable_motion_light_keyframe_t *new_keyframe, const nanoem_motion_light_keyframe_t *keyframe);

/**
 * \brief Get the opaque origin motion light keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_light_keyframe_t *APIENTRY
nanoemMutableMotionLightKeyframeGetOriginObject(nanoem_mutable_motion_light_keyframe_t *keyframe);

/**
 * \brief Serialize the opaque motion light keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeSaveToBuffer(nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion light keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeDestroy(nanoem_mutable_motion_light_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_model_keyframe Mutable Model Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion model keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion model keyframe object from the given existing object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateAsReference(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Find an opaque motion model keyframe object from the given frame index
 *
 * \param motion The opaque motion object
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionModelKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set whether the model is visible to the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetVisible(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Set the edge scale factor to the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Set the edge color to the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetEdgeColor(nanoem_mutable_motion_model_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief Set whether add blend is enabled to the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetAddBlendEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Set whether physics simulation is enabled to the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSetPhysicsSimulationEnabled(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque motion model keyframe object
 *
 * All children of the opaque motion model keyframe object are cloned and added to the destination \b new_keyframe
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeCopy(nanoem_mutable_motion_model_keyframe_t *new_keyframe, const nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Get the opaque origin motion model keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_model_keyframe_t *APIENTRY
nanoemMutableMotionModelKeyframeGetOriginObject(nanoem_mutable_motion_model_keyframe_t *keyframe);

/**
 * \brief Add the given opaque constraint state object to the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param state The opaque motion model constraint state object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status);

/**
 * \brief Remove the given opaque constraint state object from the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param state The opaque motion model constraint state object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveConstraintState(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_status_t *status);

/**
 * \brief Add the given opaque outside parent object to the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param op The opaque outside parent object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief Remove the given opaque outside parent object from the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param op The opaque outside parent object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveOutsideParent(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_outside_parent_t *op, nanoem_status_t *status);

/**
 * \brief Add the given opaque effect parameter object to the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param parameter The opaque effect parameter object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeAddEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief Remove the given opaque effect parameter object from the associated opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param parameter The opaque effect parameter object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeRemoveEffectParameter(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_status_t *status);

/**
 * \brief Serialize the opaque motion model keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeSaveToBuffer(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeDestroy(nanoem_mutable_motion_model_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_model_keyframe Mutable Model Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion model keyframe constraint state object from the given motion model keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreate(nanoem_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Serialize the opaque motion model keyframe constraint state object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateMutable(nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Create an opaque motion model keyframe constraint state object from the given existing object
 *
 * \param origin the existing opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateCreateAsReference(nanoem_motion_model_keyframe_constraint_state_t *origin, nanoem_status_t *status);

/**
 * \brief Set the bone name to the given opaque motion model keyframe constraint state object
 *
 * \param state The opaque motion model constraint state object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Set whether it's enabled to the given opaque motion model keyframe constraint state object
 *
 * \param state The opaque motion model constraint state object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_bool_t value);

/**
 * \brief Get the opaque origin motion model keyframe constraint state object from the given its object
 *
 * \param state The opaque motion model constraint state object
 */
NANOEM_DECL_API nanoem_motion_model_keyframe_constraint_state_t *APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateGetOriginObject(nanoem_mutable_motion_model_keyframe_constraint_state_t *state);

/**
 * \brief Serialize the opaque motion constraint state object and write it to the opaque mutable buffer object
 *
 * \param state The opaque motion model constraint state object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateSaveToBuffer(nanoem_mutable_motion_model_keyframe_constraint_state_t *state, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion mode keyframe constraint state object
 *
 * \param state The opaque motion model constraint state object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConstraintStateDestroy(nanoem_mutable_motion_model_keyframe_constraint_state_t *state);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_morph_keyframe Mutable Morph Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion morph keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion morph keyframe object from the given existing object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateAsReference(nanoem_motion_morph_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Find an opaque motion morph keyframe object from the given name and frame index
 *
 * \param motion The opaque motion object
 * \param name The name to find
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionMorphKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeCreateByFound(nanoem_motion_t *motion, const nanoem_unicode_string_t *name, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the weight to the given opaque motion morph keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeSetWeight(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Copy contents of the given opaque motion morph keyframe object
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeCopy(nanoem_mutable_motion_morph_keyframe_t *new_keyframe, const nanoem_motion_morph_keyframe_t *keyframe);

/**
 * \brief Get the opaque origin motion morph keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_morph_keyframe_t *APIENTRY
nanoemMutableMotionMorphKeyframeGetOriginObject(nanoem_mutable_motion_morph_keyframe_t *keyframe);

/**
 * \brief Serialize the opaque motion morph keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeSaveToBuffer(nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion morph keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeDestroy(nanoem_mutable_motion_morph_keyframe_t *keyframe);
/** @} */

/**
 * \defgroup nanoem_mutable_motion_self_shadow_keyframe Mutable Self Shadow Keyframe
 * @{
 */

/**
 * \brief Create an opaque motion self shadow keyframe object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreate(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Create an opaque motion self shadow keyframe object from the given existing object
 *
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateAsReference(nanoem_motion_self_shadow_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Find an opaque motion self shadow keyframe object from the given frame index
 *
 * \param motion The opaque motion object
 * \param index The frame index to find
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \return Same as ::nanoemMutableMotionSelfShadowKeyframeCreateAsReference if it's found, otherwise \b NULL
 */
NANOEM_DECL_API nanoem_mutable_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeCreateByFound(nanoem_motion_t *motion, nanoem_frame_index_t index, nanoem_status_t *status);

/**
 * \brief Set the distance to the given opaque motion self shadow keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetDistance(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief Set the mode to the given opaque motion self shadow keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSetMode(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, int value);

/**
 * \brief Copy contents of the given opaque motion self shadow keyframe object
 *
 * \param new_keyframe The destination opaque motion keyframe object
 * \param keyframe The source opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeCopy(nanoem_mutable_motion_self_shadow_keyframe_t *new_keyframe, const nanoem_motion_self_shadow_keyframe_t *keyframe);

/**
 * \brief Get the opaque origin motion self shadow keyframe object from the given its object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API nanoem_motion_self_shadow_keyframe_t *APIENTRY
nanoemMutableMotionSelfShadowKeyframeGetOriginObject(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe);

/**
 * \brief Serialize the opaque motion self shadow keyframe object and write it to the opaque mutable buffer object
 *
 * \param keyframe The opaque motion keyframe object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeSaveToBuffer(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque motion self shadow keyframe object
 *
 * \param keyframe The opaque motion keyframe object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeDestroy(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe);
/** @} */

/**
 * \brief Create an opaque motion object
 *
 * \param factory The opaque unicode string factory object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief Create an opaque motion object from the given existing object
 *
 * \param motion The opaque motion object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_motion_t *APIENTRY
nanoemMutableMotionCreateAsReference(nanoem_motion_t *motion, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion accessory keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion bone keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param name The name to be added
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion camera keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion light keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion model keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion morph keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param name The name to be added
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Add the given opaque motion self shadow keyframe object to the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param frame_index The frame index to be added
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAddSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion accessory keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveAccessoryKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion bone keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveBoneKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_bone_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion camera keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveCameraKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_camera_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion light keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveLightKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_light_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion model keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveModelKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion morph keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveMorphKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_morph_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Remove the given opaque motion self shadow keyframe object from the associated opaque motion object
 *
 * \param motion The opaque motion object
 * \param keyframe The opaque motion keyframe object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionRemoveSelfShadowKeyframe(nanoem_mutable_motion_t *motion, nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief Sort all motion keyframe objects by frame index order
 *
 * \param motion The opaque motion object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSortAllKeyframes(nanoem_mutable_motion_t *motion);

/**
 * \brief Set the annotation value to the given opaque motion object and the key
 *
 * \param motion The opaque motion object
 * \param key The annotation key to set
 * \param value The annotation value to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSetAnnotation(nanoem_mutable_motion_t *motion, const char *key, const char *value, nanoem_status_t *status);

/**
 * \brief Set the target model name to the given opaque motion object
 *
 * \param motion The opaque motion object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSetTargetModelName(nanoem_mutable_motion_t *motion, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin motion object from the given its object
 *
 * \param motion The opaque motion object
 */
NANOEM_DECL_API nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObject(nanoem_mutable_motion_t *motion);

/**
 * \private
 */
NANOEM_DECL_API nanoem_motion_t *APIENTRY
nanoemMutableMotionGetOriginObjectReference(nanoem_mutable_motion_t *motion);

/**
 * \brief Serialize the opaque motion object and write it to the opaque mutable buffer object
 *
 * \param motion The opaque motion object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableMotionSaveToBuffer(nanoem_mutable_motion_t *motion, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Reset all allocation size with actual motion object object size from the given opaque motion object
 *
 * \param motion The opaque motion object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionResetAllocationSize(nanoem_mutable_motion_t *motion);

/**
 * \brief Destroy the given opaque motion object
 *
 * \param motion The opaque motion object
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
 * \brief Create an opaque model vertex object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model vertex object from the given existing object
 *
 * \param vertex The opaque model vertex object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_vertex_t *APIENTRY
nanoemMutableModelVertexCreateAsReference(nanoem_model_vertex_t *vertex, nanoem_status_t *status);

/**
 * \brief Set the origin vector to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetOrigin(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the normal vector to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetNormal(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the texture coordinate vector to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetTexCoord(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the additional UV vector to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \param index The index to set
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetAdditionalUV(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value, nanoem_rsize_t index);

/**
 * \brief Set the the C value of SDEF to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefC(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the the R0 value of SDEF to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefR0(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the the R1 value of SDEF to the opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetSdefR1(nanoem_mutable_model_vertex_t *vertex, const nanoem_f32_t *value);

/**
 * \brief Set the opaque model bone object to the given opaque model vertex object and bone index
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \param index The index to set
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetBoneObject(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_bone_t *value, nanoem_rsize_t index);

/**
 * \brief Set the bone weight to the given opaque model vertex object and bone index
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 * \param index The index to set
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetBoneWeight(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value, nanoem_rsize_t index);

/**
 * \brief Set the edge size to the given opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetEdgeSize(nanoem_mutable_model_vertex_t *vertex, nanoem_f32_t value);

/**
 * \brief Set the vertex type to the given opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSetType(nanoem_mutable_model_vertex_t *vertex, nanoem_model_vertex_type_t value);

/**
 * \brief Copy contents of the given opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexCopy(nanoem_mutable_model_vertex_t *vertex, const nanoem_model_vertex_t *value);

/**
 * \brief Get the opaque origin model vertex object from the given its object
 *
 * \param vertex The opaque model vertex object
 */
NANOEM_DECL_API nanoem_model_vertex_t *APIENTRY
nanoemMutableModelVertexGetOriginObject(nanoem_mutable_model_vertex_t *vertex);

/**
 * \brief Serialize the opaque model vertex object and write it to the opaque mutable buffer object
 *
 * \param vertex The opaque model vertex object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexSaveToBuffer(nanoem_mutable_model_vertex_t *vertex, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model vertex object
 *
 * \param vertex The opaque model vertex object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelVertexDestroy(nanoem_mutable_model_vertex_t *vertex);
/** @} */

/**
 * \defgroup nanoem_mutable_model_material Mutable Material
 * @{
 */

/**
 * \brief Create an opaque model material object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model material object from the given existing object
 *
 * \param material The opaque model material object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_material_t *APIENTRY
nanoemMutableModelMaterialCreateAsReference(nanoem_model_material_t *material, nanoem_status_t *status);

/**
 * \brief Set the model material name corresponding language type
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetName(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the opaque model diffuse texture object to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief Set the opaque model sphere map texture object to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief Set the opaque model toon texture object to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonTextureObject(nanoem_mutable_model_material_t *material, nanoem_mutable_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief Set the arbitrary text object to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetClob(nanoem_mutable_model_material_t *material, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Set ambient color to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetAmbientColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief Set the diffuse color (except opacity) to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief Set the specular color to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSpecularColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief Set the edge color (except opacity) to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeColor(nanoem_mutable_model_material_t *material, const nanoem_f32_t *value);

/**
 * \brief Set the diffuse opacity to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetDiffuseOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief Set the edge opacity to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeOpacity(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief Set the edge size to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeSize(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief Set the specular power to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSpecularPower(nanoem_mutable_model_material_t *material, nanoem_f32_t value);

/**
 * \brief Set the sphere map texture type to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetSphereMapTextureType(nanoem_mutable_model_material_t *material, nanoem_model_material_sphere_map_texture_type_t value);

/**
 * \brief Set the number of vertex indices to draw the material to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetNumVertexIndices(nanoem_mutable_model_material_t *material, nanoem_rsize_t value);

/**
 * \brief Set the shared toon index between 0 and 9 to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonTextureIndex(nanoem_mutable_model_material_t *material, int value);

/**
 * \brief Set whether toon is shared to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetToonShared(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether culling is disabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCullingDisabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether casting projection shadow is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCastingShadowEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether casting shadow map is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetCastingShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether shadow map is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetShadowMapEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether edge is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetEdgeEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether vertex color is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetVertexColorEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether point drawing is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetPointDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Set whether line drawing is enabled to the given opaque model material object
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSetLineDrawEnabled(nanoem_mutable_model_material_t *material, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque model material object
 *
 * All children of the opaque model material object are cloned and added to the destination \b material
 *
 * \param material The opaque model material object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialCopy(nanoem_mutable_model_material_t *material, const nanoem_model_material_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model material object from the given its object
 *
 * \param material The opaque model material object
 */
NANOEM_DECL_API nanoem_model_material_t *APIENTRY
nanoemMutableModelMaterialGetOriginObject(nanoem_mutable_model_material_t *material);

/**
 * \brief Serialize the opaque model material object and write it to the opaque mutable buffer object
 *
 * \param material The opaque model material object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialSaveToBuffer(nanoem_mutable_model_material_t *material, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model material object
 *
 * \param material The opaque model material object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMaterialDestroy(nanoem_mutable_model_material_t *material);
/** @} */

/**
 * \defgroup nanoem_mutable_model_bone Mutable Bone
 * @{
 */

/**
 * \brief Create an opaque model bone object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model bone object from the given existing object
 *
 * \param bone The opaque model bone object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_bone_t *APIENTRY
nanoemMutableModelBoneCreateAsReference(nanoem_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief Set the model bone name corresponding language type
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetName(nanoem_mutable_model_bone_t *bone, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the parent bone object to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief Set the parent inherent bone object to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentParentBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief Set the effector bone object to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetEffectorBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief Set the target bone object to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetTargetBoneObject(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value);

/**
 * \brief Set the opaque constraint object to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value);

/**
 * \brief Remove the given opaque model constraint object from the associated opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value The opaque model constraint object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneRemoveConstraintObject(nanoem_mutable_model_bone_t *bone, nanoem_mutable_model_constraint_t *value);

/**
 * \brief Set the origin vector to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief Set the destination origin vector to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetDestinationOrigin(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief Set the fixed axis vector to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetFixedAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief Set the X local axis vector to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalXAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief Set the Z local axis vector to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalZAxis(nanoem_mutable_model_bone_t *bone, const nanoem_f32_t *value);

/**
 * \brief Set the inherent coefficient to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentCoefficient(nanoem_mutable_model_bone_t *bone, nanoem_f32_t value);

/**
 * \brief Set the stage index to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetStageIndex(nanoem_mutable_model_bone_t *bone, int value);

/**
 * \private
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetOffsetRelative(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone can rotate to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetRotateable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone can move to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetMovable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone is visible to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetVisible(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone can handle from user to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetUserHandleable(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone has constraint to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetConstraintEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone has local inherent to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalInherentEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the inherent translation is enabled to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentTranslationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the inherent orientation is enabled to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetInherentOrientationEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone has fixed axis to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetFixedAxisEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone has local axes to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetLocalAxesEnabled(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone is affected by physics simulation to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSetAffectedByPhysicsSimulation(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Set whether the bone has external parent bone to the given opaque model bone object
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneEnableExternalParentBone(nanoem_mutable_model_bone_t *bone, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque model bone object
 *
 * All children of the opaque model bone object are cloned and added to the destination \b bone
 *
 * \param bone The opaque model bone object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneCopy(nanoem_mutable_model_bone_t *bone, const nanoem_model_bone_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model bone object from the given its object
 *
 * \param bone The opaque model bone object
 */
NANOEM_DECL_API nanoem_model_bone_t *APIENTRY
nanoemMutableModelBoneGetOriginObject(nanoem_mutable_model_bone_t *bone);

/**
 * \brief Serialize the opaque model bone object and write it to the opaque mutable buffer object
 *
 * \param bone The opaque model bone object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneSaveToBuffer(nanoem_mutable_model_bone_t *bone, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model bone object
 *
 * \param bone The opaque model bone object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelBoneDestroy(nanoem_mutable_model_bone_t *bone);
/** @} */

/**
 * \defgroup nanoem_mutable_model_constraint Mutable Constraint
 * @{
 */

/**
 * \brief Create an opaque model constraint object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model constraint object from the given existing object
 *
 * \param constraint The opaque model constraint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_t *APIENTRY
nanoemMutableModelConstraintCreateAsReference(nanoem_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief Set the effector bone to the given opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetEffectorBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value);

/**
 * \brief Set the target bone to the given opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetTargetBoneObject(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_bone_t *value);

/**
 * \brief Set the angle limit in radians to the given opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetAngleLimit(nanoem_mutable_model_constraint_t *constraint, nanoem_f32_t value);

/**
 * \brief Set the max number of iterations to the given opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSetNumIterations(nanoem_mutable_model_constraint_t *constraint, int value);

/**
 * \brief Insert an opaque constraint joint object into the opaque constraint model joint
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintInsertJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, int index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model constraint joint object from the associated opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 * \param value The opaque model constraint joint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintRemoveJointObject(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_model_constraint_joint_t *value, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque model constraint object
 *
 * All children of the opaque model constraint object are cloned and added to the destination \b constraint
 *
 * \param constraint The opaque model constraint object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintCopy(nanoem_mutable_model_constraint_t *constraint, const nanoem_model_constraint_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model constraint object from the given its object
 *
 * \param constraint The opaque model constraint object
 */
NANOEM_DECL_API nanoem_model_constraint_t *APIENTRY
nanoemMutableModelConstraintGetOriginObject(nanoem_mutable_model_constraint_t *constraint);

/**
 * \brief Serialize the opaque model constraint object and write it to the opaque mutable buffer object
 *
 * \param constraint The opaque model constraint object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintSaveToBuffer(nanoem_mutable_model_constraint_t *constraint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model constraint object
 *
 * \param constraint The opaque model constraint object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintDestroy(nanoem_mutable_model_constraint_t *constraint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_constraint_joint Mutable Constraint Joint
 * @{
 */

/**
 * \brief Create an opaque model constraint joint object
 *
 * \param constraint The opaque model constraint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreate(nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief Create an opaque model constraint joint object from the given existing object
 *
 * \param joint The opaque model joint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointCreateAsReference(nanoem_model_constraint_joint_t *joint, nanoem_status_t *status);

/**
 * \brief Set the opaque model bone object to the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetBoneObject(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_bone_t *value);

/**
 * \brief Set the upper limit angles in radians to the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetUpperLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the lower limit angles in radians to the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetLowerLimit(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set whether angle limit is set to the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSetAngleLimitEnabled(nanoem_mutable_model_constraint_joint_t *joint, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointCopy(nanoem_mutable_model_constraint_joint_t *joint, const nanoem_model_constraint_joint_t *value);

/**
 * \brief Get the opaque origin model constraint joint object from the given its object
 *
 * \param joint The opaque model joint object
 */
NANOEM_DECL_API nanoem_model_constraint_joint_t *APIENTRY
nanoemMutableModelConstraintJointGetOriginObject(nanoem_mutable_model_constraint_joint_t *joint);

/**
 * \brief Serialize the opaque model constraint joint object and write it to the opaque mutable buffer object
 *
 * \param joint The opaque model joint object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointSaveToBuffer(nanoem_mutable_model_constraint_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model constraint joint object
 *
 * \param joint The opaque model joint object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelConstraintJointDestroy(nanoem_mutable_model_constraint_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_texture Mutable Texture
 * @{
 */

/**
 * \brief Create an opaque model texture object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model texture object from the given existing object
 *
 * \param texture The opaque model texture object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_texture_t *APIENTRY
nanoemMutableModelTextureCreateAsReference(nanoem_model_texture_t *texture, nanoem_status_t *status);

/**
 * \brief Set the relative texture path unicode string object to the given opaque model texture object
 *
 * \param texture The opaque model texture object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureSetPath(nanoem_mutable_model_texture_t *texture, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque model texture object
 *
 * All children of the opaque model texture object are cloned and added to the destination \b texture
 *
 * \param texture The opaque model texture object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureCopy(nanoem_mutable_model_texture_t *texture, const nanoem_model_texture_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model texture object from the given its object
 *
 * \param texture The opaque model texture object
 */
NANOEM_DECL_API nanoem_model_texture_t *APIENTRY
nanoemMutableModelTextureGetOriginObject(nanoem_mutable_model_texture_t *texture);

/**
 * \brief Serialize the opaque model texture object and write it to the opaque mutable buffer object
 *
 * \param texture The opaque model texture object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelTextureSaveToBuffer(nanoem_mutable_model_texture_t *texture, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model texture object
 *
 * \param texture The opaque model texture object
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
 * \brief Create an opaque model morph bone object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph bone object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneCreateAsReference(nanoem_model_morph_bone_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque model bone object to the given opaque model morph bone object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetBoneObject(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_bone_t *value);

/**
 * \brief Set the translation vector to the given opaque model morph bone object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetTranslation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the orientation quaternion to the given opaque model morph bone object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSetOrientation(nanoem_mutable_model_morph_bone_t *morph, const nanoem_f32_t *value);

/**
 * \brief Copy contents of the given opaque model morph bone object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneCopy(nanoem_mutable_model_morph_bone_t *morph, const nanoem_model_morph_bone_t *value);

/**
 * \brief Serialize the opaque model bone object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneSaveToBuffer(nanoem_mutable_model_morph_bone_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph bone object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_bone_t *APIENTRY
nanoemMutableModelMorphBoneGetOriginObject(nanoem_mutable_model_morph_bone_t *morph);

/**
 * \brief Destroy the given opaque model morph bone object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphBoneDestroy(nanoem_mutable_model_morph_bone_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_flip Mutable Flip Morph
 * @{
 */

/**
 * \brief Create an opaque model morph flip object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph flip object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipCreateAsReference(nanoem_model_morph_flip_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque morph object to the given opaque model morph flip object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSetMorphObject(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_t *value);

/**
 * \brief Set the weight to the given opaque model morph flip object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSetWeight(nanoem_mutable_model_morph_flip_t *morph, nanoem_f32_t value);

/**
 * \brief Copy contents of the given opaque model morph flip object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipCopy(nanoem_mutable_model_morph_flip_t *morph, const nanoem_model_morph_flip_t *value);

/**
 * \brief Serialize the opaque model morph flip object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipSaveToBuffer(nanoem_mutable_model_morph_flip_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph flip object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_flip_t *APIENTRY
nanoemMutableModelMorphFlipGetOriginObject(nanoem_mutable_model_morph_flip_t *morph);

/**
 * \brief Destroy the given opaque model morph flip object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphFlipDestroy(nanoem_mutable_model_morph_flip_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_group Mutable Group Morph
 * @{
 */

/**
 * \brief Create an opaque model morph group object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph group object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupCreateAsReference(nanoem_model_morph_group_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque morph object to the given opaque model morph group object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSetMorphObject(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_t *value);

/**
 * \brief Set the weight to the given opaque model morph flip object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSetWeight(nanoem_mutable_model_morph_group_t *morph, nanoem_f32_t value);

/**
 * \brief Copy contents of the given opaque model morph group object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupCopy(nanoem_mutable_model_morph_group_t *morph, const nanoem_model_morph_group_t *value);

/**
 * \brief Serialize the opaque model morph group object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupSaveToBuffer(nanoem_mutable_model_morph_group_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph group object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_group_t *APIENTRY
nanoemMutableModelMorphGroupGetOriginObject(nanoem_mutable_model_morph_group_t *morph);

/**
 * \brief Destroy the given opaque model morph group object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphGroupDestroy(nanoem_mutable_model_morph_group_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_impulse Mutable Impulse Morph
 * @{
 */

/**
 * \brief Create an opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph impulse object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseCreateAsReference(nanoem_model_morph_impulse_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque rigid body object to the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetRigidBodyObject(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_rigid_body_t *value);

/**
 * \brief Set the torque vector to the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetTorque(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the velocity vector to the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetVelocity(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set whether it's local coordination to the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSetLocal(nanoem_mutable_model_morph_impulse_t *morph, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseCopy(nanoem_mutable_model_morph_impulse_t *morph, const nanoem_model_morph_impulse_t *value);

/**
 * \brief Serialize the opaque model morph impulse object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseSaveToBuffer(nanoem_mutable_model_morph_impulse_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph impulse object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_impulse_t *APIENTRY
nanoemMutableModelMorphImpulseGetOriginObject(nanoem_mutable_model_morph_impulse_t *morph);

/**
 * \brief Destroy the given opaque model morph impulse object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphImpulseDestroy(nanoem_mutable_model_morph_impulse_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_material Mutable Material Morph
 * @{
 */

/**
 * \brief Create an opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph material object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialCreateAsReference(nanoem_model_morph_material_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque model material object to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetMaterialObject(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_material_t *value);

/**
 * \brief Set the ambient color to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetAmbientColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the diffuse color (except opacity) to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the specular color to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the edge color (except opacity) to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeColor(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the diffuse texture blend factor to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the sphere map texture blend factor to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSphereMapTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the toon texture blend factor to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetToonTextureBlend(nanoem_mutable_model_morph_material_t *morph, const nanoem_f32_t *value);

/**
 * \brief Set the diffuse opacity to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetDiffuseOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief Set the edge opacity to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeOpacity(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief Set the specular power to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetSpecularPower(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief Set the edge size to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetEdgeSize(nanoem_mutable_model_morph_material_t *morph, nanoem_f32_t value);

/**
 * \brief Set the operation type to the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSetOperationType(nanoem_mutable_model_morph_material_t *morph, nanoem_model_morph_material_operation_type_t value);

/**
 * \brief Copy contents of the given opaque model material object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialCopy(nanoem_mutable_model_morph_material_t *morph, const nanoem_model_morph_material_t *value);

/**
 * \brief Serialize the opaque model morph material object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialSaveToBuffer(nanoem_mutable_model_morph_material_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph material object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_material_t *APIENTRY
nanoemMutableModelMorphMaterialGetOriginObject(nanoem_mutable_model_morph_material_t *morph);

/**
 * \brief Destroy the given opaque model morph material object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphMaterialDestroy(nanoem_mutable_model_morph_material_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_uv Mutable UV Morph
 * @{
 */

/**
 * \brief Create an opaque model morph UV object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph UV object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVCreateAsReference(nanoem_model_morph_uv_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque model vertex object to the given opaque model morph UV object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSetVertexObject(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_vertex_t *value);

/**
 * \brief Set the position vector to the given opaque model morph UV object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSetPosition(nanoem_mutable_model_morph_uv_t *morph, const nanoem_f32_t *value);

/**
 * \brief Copy contents of the given opaque model morph UV object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVCopy(nanoem_mutable_model_morph_uv_t *morph, const nanoem_model_morph_uv_t *value);

/**
 * \brief Serialize the opaque model morph UV object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVSaveToBuffer(nanoem_mutable_model_morph_uv_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph UV object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_uv_t *APIENTRY
nanoemMutableModelMorphUVGetOriginObject(nanoem_mutable_model_morph_uv_t *morph);

/**
 * \brief Destroy the given opaque model morph UV object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphUVDestroy(nanoem_mutable_model_morph_uv_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_morph_vertex Mutable Vertex Morph
 * @{
 */

/**
 * \brief Create an opaque model morph vertex object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreate(nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph vertex object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexCreateAsReference(nanoem_model_morph_vertex_t *morph, nanoem_status_t *status);

/**
 * \brief Set the opaque model vertex object to the given opaque model morph vertex object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSetVertexObject(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_vertex_t *value);

/**
 * \brief Set the position vector to the given opaque model morph vertex object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSetPosition(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_f32_t *value);

/**
 * \brief Copy contents of the given opaque model morph vertex object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexCopy(nanoem_mutable_model_morph_vertex_t *morph, const nanoem_model_morph_vertex_t *value);

/**
 * \brief Serialize the opaque model morph vertex object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexSaveToBuffer(nanoem_mutable_model_morph_vertex_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph vertex object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_vertex_t *APIENTRY
nanoemMutableModelMorphVertexGetOriginObject(nanoem_mutable_model_morph_vertex_t *morph);

/**
 * \brief Destroy the given opaque model morph vertex object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphVertexDestroy(nanoem_mutable_model_morph_vertex_t *morph);
/** @} */

/**
 * \brief Create an opaque model morph object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model morph object from the given existing object
 *
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_morph_t *APIENTRY
nanoemMutableModelMorphCreateAsReference(nanoem_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Set the model morph name corresponding language type
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetName(nanoem_mutable_model_morph_t *morph, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the category to the given opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetCategory(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_category_t value);

/**
 * \brief Set the type to the given opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSetType(nanoem_mutable_model_morph_t *morph, nanoem_model_morph_type_t value);

/**
 * \brief Insert an opaque model morph bone object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph flip object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph group object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph impulse object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph material object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph UV object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph vertex object into the opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value value to set to the opaque object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphInsertVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, int index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph bone object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph bone object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveBoneMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_bone_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph flip object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph flip object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveFlipMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_flip_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph group object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph group object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveGroupMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_group_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph impulse object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph impulse object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveImpulseMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_impulse_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph material object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph material object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveMaterialMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_material_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph UV object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph UV object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveUVMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_uv_t *value, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph vertex object from the associated opaque model morph object
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph vertex object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphRemoveVertexMorphObject(nanoem_mutable_model_morph_t *morph, nanoem_mutable_model_morph_vertex_t *value, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque model morph object
 *
 * All children of the opaque model morph object are cloned and added to the destination \b morph
 *
 * \param morph The opaque model morph object
 * \param value The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphCopy(nanoem_mutable_model_morph_t *morph, const nanoem_model_morph_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model morph object from the given its object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API nanoem_model_morph_t *APIENTRY
nanoemMutableModelMorphGetOriginObject(nanoem_mutable_model_morph_t *morph);

/**
 * \brief Serialize the opaque model morph object and write it to the opaque mutable buffer object
 *
 * \param morph The opaque model morph object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphSaveToBuffer(nanoem_mutable_model_morph_t *morph, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model morph object
 *
 * \param morph The opaque model morph object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelMorphDestroy(nanoem_mutable_model_morph_t *morph);
/** @} */

/**
 * \defgroup nanoem_mutable_model_label Mutable Label
 * @{
 */

/**
 * \brief Create an opaque model label object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model label object from the given existing object
 *
 * \param label The opaque model label object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_label_t *APIENTRY
nanoemMutableModelLabelCreateAsReference(nanoem_model_label_t *label, nanoem_status_t *status);

/**
 * \brief Set the model label name corresponding language type
 *
 * \param label The opaque model label object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSetName(nanoem_mutable_model_label_t *label, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set whether the label is special to the given opaque model label object
 *
 * \param label The opaque model label object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSetSpecial(nanoem_mutable_model_label_t *label, nanoem_bool_t value);

/**
 * \brief Copy contents of the given opaque model label object
 *
 * All children of the opaque model label object are cloned and added to the destination \b label
 *
 * \param label The opaque model label object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelCopy(nanoem_mutable_model_label_t *label, const nanoem_model_label_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model label object from the given its object
 *
 * \param label The opaque model label object
 */
NANOEM_DECL_API nanoem_model_label_t *APIENTRY
nanoemMutableModelLabelGetOriginObject(nanoem_mutable_model_label_t *label);

/**
 * \brief Serialize the opaque model label object and write it to the opaque mutable buffer object
 *
 * \param label The opaque model label object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelSaveToBuffer(nanoem_mutable_model_label_t *label, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model label object
 *
 * \param label The opaque model label object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelDestroy(nanoem_mutable_model_label_t *label);

/**
 * \defgroup nanoem_mutable_model_label_item Mutable Label Item
 * @{
 */

/**
 * \brief Create an opaque model label item object
 *
 * \param item The opaque model label item object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateAsReference(nanoem_model_label_item_t *item, nanoem_status_t *status);

/**
 * \brief Create an opaque model label item object from the given opaque model bone object
 *
 * \param label The opaque model label object
 * \param bone The opaque model bone object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromBoneObject(nanoem_mutable_model_label_t *label, nanoem_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief Create an opaque model vertex object from the given opaque model morph object
 *
 * \param label The opaque model label object
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemCreateFromMorphObject(nanoem_mutable_model_label_t *label, nanoem_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Insert an opaque model label item object into the opaque model label object
 *
 * \param label The opaque model label object
 * \param item The opaque model label item object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelInsertItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, int index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model label item object from the associated opaque model label object
 *
 * \param label The opaque model label object
 * \param item The opaque model label item object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelRemoveItemObject(nanoem_mutable_model_label_t *label, nanoem_mutable_model_label_item_t *item, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model label item object from the given its object
 *
 * \param label The opaque model label object
 */
NANOEM_DECL_API nanoem_model_label_item_t *APIENTRY
nanoemMutableModelLabelItemGetOriginObject(nanoem_mutable_model_label_item_t *label);

/**
 * \brief Destroy the given opaque model label item object
 *
 * \param item The opaque model label item object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelLabelItemDestroy(nanoem_mutable_model_label_item_t *item);
/** @} */

/** @} */

/**
 * \defgroup nanoem_mutable_model_rigid_body Mutable Rigid Body
 * @{
 */

/**
 * \brief Create an opaque model rigid body object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model rigid body object from the given existing object
 *
 * \param rigid_body The opaque model rigid body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyCreateAsReference(nanoem_model_rigid_body_t *rigid_body, nanoem_status_t *status);

/**
 * \brief Set the model rigid body name corresponding language type
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetName(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the opaque model bone object to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetBoneObject(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_bone_t *value);

/**
 * \brief Set the origin vector to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetOrigin(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Set the orientation euler angles vector in radians to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetOrientation(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Set the shape size vector to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetShapeSize(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_f32_t *value);

/**
 * \brief Set the mass to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetMass(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief Set the linear damping vector to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetLinearDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief Set the angular damping vector to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetAngularDamping(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief Set the friction to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetFriction(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief Set the restitution to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetRestitution(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_f32_t value);

/**
 * \brief Set the shape type to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetShapeType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_shape_type_t value);

/**
 * \brief Set the transform type to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetTransformType(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_model_rigid_body_transform_type_t value);

/**
 * \brief Set the collision ID between 0 and 15 to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetCollisionGroupId(nanoem_mutable_model_rigid_body_t *rigid_body, int value);

/**
 * \brief Set the collision group mask to the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySetCollisionMask(nanoem_mutable_model_rigid_body_t *rigid_body, int value);

/**
 * \brief Copy contents of the given opaque model rigid body object
 *
 * All children of the opaque model rigid body object are cloned and added to the destination \b rigid_body
 *
 * \param rigid_body The opaque model rigid body object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodyCopy(nanoem_mutable_model_rigid_body_t *rigid_body, const nanoem_model_rigid_body_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model rigid body object from the given its object
 *
 * \param rigid_body The opaque model rigid body object
 */
NANOEM_DECL_API nanoem_model_rigid_body_t *APIENTRY
nanoemMutableModelRigidBodyGetOriginObject(nanoem_mutable_model_rigid_body_t *rigid_body);

/**
 * \brief Serialize the opaque model rigid body object and write it to the opaque mutable buffer object
 *
 * \param rigid_body The opaque model rigid body object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodySaveToBuffer(nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model rigid body object
 *
 * \param rigid_body The opaque model rigid body object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRigidBodyDestroy(nanoem_mutable_model_rigid_body_t *rigid_body);
/** @} */

/**
 * \defgroup nanoem_mutable_model_joint Mutable Joint
 * @{
 */

/**
 * \brief Create an opaque model joint object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model joint object from the given existing object
 *
 * \param joint The opaque model joint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_joint_t *APIENTRY
nanoemMutableModelJointCreateAsReference(nanoem_model_joint_t *joint, nanoem_status_t *status);

/**
 * \brief Set the model joint name corresponding language type
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetName(nanoem_mutable_model_joint_t *joint, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the opaque model rigid body object to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetRigidBodyAObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value);

/**
 * \brief Set the opaque model rigid body object to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetRigidBodyBObject(nanoem_mutable_model_joint_t *joint, const nanoem_model_rigid_body_t *value);

/**
 * \brief Set the origin vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetOrigin(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the orientation euler angles vector in radians to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetOrientation(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the type to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetType(nanoem_mutable_model_joint_t *joint, nanoem_model_joint_type_t value);

/**
 * \brief Set the linear upper limit vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the linear lower limit vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the linear stiffness vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetLinearStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the angular upper limit vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularUpperLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the angular lower limit vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularLowerLimit(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Set the angular stiffness vector to the given opaque model joint object
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \remark \b value must be at least 4 components float array
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSetAngularStiffness(nanoem_mutable_model_joint_t *joint, const nanoem_f32_t *value);

/**
 * \brief Copy contents of the given opaque model joint object
 *
 * All children of the opaque model joint object are cloned and added to the destination \b joint
 *
 * \param joint The opaque model joint object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointCopy(nanoem_mutable_model_joint_t *joint, const nanoem_model_joint_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model joint object from the given its object
 *
 * \param joint The opaque model joint object
 */
NANOEM_DECL_API nanoem_model_joint_t *APIENTRY
nanoemMutableModelJointGetOriginObject(nanoem_mutable_model_joint_t *joint);

/**
 * \brief Serialize the opaque model joint object and write it to the opaque mutable buffer object
 *
 * \param joint The opaque model joint object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointSaveToBuffer(nanoem_mutable_model_joint_t *joint, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model joint object
 *
 * \param joint The opaque model joint object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelJointDestroy(nanoem_mutable_model_joint_t *joint);
/** @} */

/**
 * \defgroup nanoem_mutable_model_softbody Mutable Soft Body
 * @{
 */

/**
 * \brief Create an opaque model soft body object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreate(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Create an opaque model soft body object from the given existing object
 *
 * \param soft_body The opaque model soft body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyCreateAsReference(nanoem_model_soft_body_t *soft_body, nanoem_status_t *status);

/**
 * \brief Set the model soft body name corresponding language type
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetName(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Get the opaque model material object from the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetMaterialObject(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_model_material_t *value);

/**
 * \brief Set all pinned vertex indices to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 * \param length size of \b value
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAllPinnedVertexIndices(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status);

/**
 * \brief Set the shape type to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetShapeType(nanoem_mutable_model_soft_body_t *soft_body, nanoem_model_soft_body_shape_type_t value);

/**
 * \brief Set the aero model to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAeroModel(nanoem_mutable_model_soft_body_t *soft_body, nanoem_model_soft_body_aero_model_type_t value);

/**
 * \brief Set the total mass to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetTotalMass(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the collision margin to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionMargin(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the velocity correction factor ( \b kVCF ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVelocityCorrectionFactor(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the damping coefficient ( \b kDP ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDampingCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the drag coefficient ( \b kDG ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDragCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the lift coefficient ( \b kLF ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetLiftCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the pressure coefficient ( \b kPR ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPressureCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the volume conversation coefficient  ( \b kVC ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVolumeConversationCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the dynamic friction coefficient ( \b kDF ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDynamicFrictionCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the pose matching coefficient ( \b kMT ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPoseMatchingCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the rigid contact hardness ( \b kCHR ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetRigidContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the kinetic contact hardness ( \b kKHR ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetKineticContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft contact hardness ( \b kSHR ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftContactHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the anchor hardness ( \b kAHR ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAnchorHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs rigid hardness ( \b kSRHR_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs kinetic hardness ( \b kSKHR_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs soft hardness ( \b kSSHR_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftHardness(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs rigid impulse split ( \b kSR_SPLT_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSRigidImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs kinetic impulse split ( \b kSK_SPLT_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSKineticImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the soft vs soft impulse split ( \b kSS_SPLT_CL ) to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetSoftVSSoftImpulseSplit(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the linear stiffness coefficient to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetLinearStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the angular stiffness coefficient to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetAngularStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the volume stiffness coefficient to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVolumeStiffnessCoefficient(nanoem_mutable_model_soft_body_t *soft_body, nanoem_f32_t value);

/**
 * \brief Set the collision group ID between 0 and 15 to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionGroupId(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the collision group mask to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetCollisionMask(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the bending constraints distance to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsDistance(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the cluster count to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClusterCount(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the velocity solver iterations to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetVelocitySolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the position solver iterations to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetPositionsSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the drift solver iterations to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetDriftSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set the cluster solver iterations to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClusterSolverIterations(nanoem_mutable_model_soft_body_t *soft_body, int value);

/**
 * \brief Set whether bending constraint is enabled to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetBendingConstraintsEnabled(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value);

/**
 * \brief Set whether cluster is enabled to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetClustersEnabled(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value);

/**
 * \brief Set whether randomized constraint is needed to the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySetRandomizeConstraintsNeeded(nanoem_mutable_model_soft_body_t *soft_body, nanoem_bool_t value);

/**
 * \brief Insert an opaque model soft body anchor object into the opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param anchor The opaque model soft body anchor object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyInsertAnchorObject(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_model_soft_body_anchor_t *anchor, int index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque soft body anchor object from the associated opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 * \param anchor The opaque model soft body anchor object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyRemoveAnchorObject(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_status_t *status);

/**
 * \brief Copy contents of the given opaque model soft body object
 *
 * All children of the opaque model soft body object are cloned and added to the destination \b soft_body
 *
 * \param soft_body The opaque model soft body object
 * \param value value to set to the opaque object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyCopy(nanoem_mutable_model_soft_body_t *soft_body, const nanoem_model_soft_body_t *value, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model soft body object from the given its object
 *
 * \param soft_body The opaque model soft body object
 */
NANOEM_DECL_API nanoem_model_soft_body_t *APIENTRY
nanoemMutableModelSoftBodyGetOriginObject(nanoem_mutable_model_soft_body_t *soft_body);

/**
 * \brief Serialize the opaque model soft body object and write it to the opaque mutable buffer object
 *
 * \param soft_body The opaque model soft body object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodySaveToBuffer(nanoem_mutable_model_soft_body_t *soft_body, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Destroy the given opaque model soft body object
 *
 * \param soft_body The opaque model soft body object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyDestroy(nanoem_mutable_model_soft_body_t *soft_body);

/**
 * \defgroup nanoem_mutable_model_softbody_anchor Mutable Soft Body Anchor
 * @{
 */

/**
 * \brief Create an opaque model soft body anchor object
 *
 * \param soft_body The opaque model soft body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreate(nanoem_mutable_model_soft_body_t *soft_body, nanoem_status_t *status);

/**
 * \brief Create an opaque model soft body object from the given existing object
 *
 * \param anchor The opaque model soft body anchor object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorCreateAsReference(nanoem_model_soft_body_anchor_t *anchor, nanoem_status_t *status);

/**
 * \brief Set the opaque model soft body object to the given opaque model soft body anchor object
 *
 * \param anchor The opaque model soft body anchor object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetRigidBodyObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_rigid_body_t *value);

/**
 * \brief Set the opaque model vertex object to the given opaque model soft body anchor object
 *
 * \param anchor The opaque model soft body anchor object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetVertexObject(nanoem_mutable_model_soft_body_anchor_t *anchor, const nanoem_model_vertex_t *value);

/**
 * \brief Set whether near mode is enabled to the given opaque model soft body anchor object
 *
 * \param anchor The opaque model soft body anchor object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorSetNearEnabled(nanoem_mutable_model_soft_body_anchor_t *anchor, nanoem_bool_t value);

/**
 * \brief Get the opaque origin model soft body anchor object from the given its object
 *
 * \param anchor The opaque model soft body anchor object
 */
NANOEM_DECL_API nanoem_model_soft_body_anchor_t *APIENTRY
nanoemMutableModelSoftBodyAnchorGetOriginObject(nanoem_mutable_model_soft_body_anchor_t *anchor);

/**
 * \brief Destroy the given opaque model soft body anchor object
 *
 * \param anchor The opaque model soft body anchor object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSoftBodyAnchorDestroy(nanoem_mutable_model_soft_body_anchor_t *anchor);
/** @} */

/** @} */

/**
 * \brief Create an opaque model object
 *
 * \param factory The opaque unicode string factory object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief Create an opaque model object from the given existing object
 *
 * \param model The opaque model object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_mutable_model_t *APIENTRY
nanoemMutableModelCreateAsReference(nanoem_model_t *model, nanoem_status_t *status);

/**
 * \brief Set the format type to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetFormatType(nanoem_mutable_model_t *model, nanoem_model_format_type_t value);

/**
 * \brief Set the codec type to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetCodecType(nanoem_mutable_model_t *model, nanoem_codec_type_t value);

/**
 * \brief Set the additional UV size to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetAdditionalUVSize(nanoem_mutable_model_t *model, nanoem_rsize_t value);

/**
 * \brief Set the name corresponding language type to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetName(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set the comment corresponding language type to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 * \param language The language to set
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetComment(nanoem_mutable_model_t *model, const nanoem_unicode_string_t *value, nanoem_language_type_t language, nanoem_status_t *status);

/**
 * \brief Set all model vertex indices to the given opaque model object
 *
 * \param model The opaque model object
 * \param value value to set to the opaque object
 * \param length size of \b value
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelSetVertexIndices(nanoem_mutable_model_t *model, const nanoem_u32_t *value, nanoem_rsize_t length, nanoem_status_t *status);

/**
 * \brief Insert an opaque model vertex object into the opaque model object
 *
 * \param model The opaque model object
 * \param vertex The opaque model vertex object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model material object into the opaque model object
 *
 * \param model The opaque model object
 * \param material The opaque model material object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model bone object into the opaque model object
 *
 * \param model The opaque model object
 * \param bone The opaque model bone object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model constraint object into the opaque model object
 *
 * \param model The opaque model object
 * \param constraint The opaque model constraint object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model texture object into the opaque model object
 *
 * \param model The opaque model object
 * \param texture The opaque model texture object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model morph object into the opaque model object
 *
 * \param model The opaque model object
 * \param morph The opaque model morph object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model label object into the opaque model object
 *
 * \param model The opaque model object
 * \param label The opaque model label object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model rigid body object into the opaque model object
 *
 * \param model The opaque model object
 * \param rigid_body The opaque model rigid body object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model joint object into the opaque model object
 *
 * \param model The opaque model object
 * \param joint The opaque model joint object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, int index, nanoem_status_t *status);

/**
 * \brief Insert an opaque model soft body object into the opaque model object
 *
 * \param model The opaque model object
 * \param soft_body The opaque model soft body object
 * \param index The index offset to insert, specifing \b -1 will be appended
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelInsertSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, int index, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model vertex object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param vertex The opaque model vertex object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveVertexObject(nanoem_mutable_model_t *model, nanoem_mutable_model_vertex_t *vertex, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model material object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param material The opaque model material object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveMaterialObject(nanoem_mutable_model_t *model, nanoem_mutable_model_material_t *material, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model bone object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param bone The opaque model bone object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveBoneObject(nanoem_mutable_model_t *model, nanoem_mutable_model_bone_t *bone, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model constraint object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param constraint The opaque model constraint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveConstraintObject(nanoem_mutable_model_t *model, nanoem_mutable_model_constraint_t *constraint, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model texture object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param texture The opaque model texture object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveTextureObject(nanoem_mutable_model_t *model, nanoem_mutable_model_texture_t *texture, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model morph object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param morph The opaque model morph object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveMorphObject(nanoem_mutable_model_t *model, nanoem_mutable_model_morph_t *morph, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model label object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param label The opaque model label object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveLabelObject(nanoem_mutable_model_t *model, nanoem_mutable_model_label_t *label, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model rigid body object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param rigid_body The opaque model rigid body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveRigidBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_rigid_body_t *rigid_body, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model joint object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param joint The opaque model joint object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveJointObject(nanoem_mutable_model_t *model, nanoem_mutable_model_joint_t *joint, nanoem_status_t *status);

/**
 * \brief Remove the given opaque model soft body object from the associated opaque model object
 *
 * \param model The opaque model object
 * \param soft_body The opaque model soft body object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelRemoveSoftBodyObject(nanoem_mutable_model_t *model, nanoem_mutable_model_soft_body_t *soft_body, nanoem_status_t *status);

/**
 * \brief Get the opaque origin model object from the given its object
 *
 * \param model The opaque model object
 */
NANOEM_DECL_API nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObject(nanoem_mutable_model_t *model);

/**
 * \private
 */
NANOEM_DECL_API nanoem_model_t *APIENTRY
nanoemMutableModelGetOriginObjectReference(nanoem_mutable_model_t *model);

/**
 * \brief Serialize the opaque model object and write it to the opaque mutable buffer object
 *
 * \param model The opaque model object
 * \param buffer The opaque buffer object
 * \param[in,out] status \b NANOEM_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableModelSaveToBuffer(nanoem_mutable_model_t *model, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief Reset all allocation size with actual model object object size from the given opaque model object
 *
 * \param model The opaque model object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelResetAllocationSize(nanoem_mutable_model_t *model);

/**
 * \brief Destroy the given opaque model object
 *
 * \param model The opaque model object
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableModelDestroy(nanoem_mutable_model_t *model);
/** @} */

#endif /* NANOEM_EXT_MUTABLE_H_ */
