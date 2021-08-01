/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_DOCUMENT_H_
#define NANOEM_EXT_DOCUMENT_H_

#include "../nanoem.h"
#include "./mutable.h"

/**
 * \defgroup nanoem nanoem
 * @{
 */

/**
 * \defgroup nanoem_document Document
 * @{
 */

NANOEM_DECL_OPAQUE(nanoem_document_base_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_outside_parent_t);
NANOEM_DECL_OPAQUE(nanoem_document_accessory_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_bone_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_camera_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_gravity_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_light_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_morph_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_self_shadow_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_bone_state_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_morph_state_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_constraint_state_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_t);
NANOEM_DECL_OPAQUE(nanoem_document_camera_t);
NANOEM_DECL_OPAQUE(nanoem_document_light_t);
NANOEM_DECL_OPAQUE(nanoem_document_accessory_t);
NANOEM_DECL_OPAQUE(nanoem_document_gravity_t);
NANOEM_DECL_OPAQUE(nanoem_document_self_shadow_t);
NANOEM_DECL_OPAQUE(nanoem_document_model_outside_parent_state_t);
NANOEM_DECL_OPAQUE(nanoem_document_t);

typedef nanoem_model_t *(*nanoem_document_parse_model_callback_t)(void *, const nanoem_unicode_string_t *, nanoem_unicode_string_factory_t *, nanoem_status_t *);

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_document_editing_mode_t) {
    NANOEM_DOCUMENT_EDITING_MODE_FIRST_ENUM,
    NANOEM_DOCUMENT_EDITING_MODE_SELECT = NANOEM_DOCUMENT_EDITING_MODE_FIRST_ENUM,
    NANOEM_DOCUMENT_EDITING_MODE_SELECT_BOX,
    NANOEM_DOCUMENT_EDITING_MODE_NONE,
    NANOEM_DOCUMENT_EDITING_MODE_ROTATE,
    NANOEM_DOCUMENT_EDITING_MODE_MOVE,
    NANOEM_DOCUMENT_EDITING_MODE_MAX_ENUM,
};

NANOEM_DECL_ENUM(nanoem_i32_t, nanoem_document_physics_simulation_mode_t){
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_FIRST_ENUM,
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_DISABLE = NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_FIRST_ENUM,
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_ANYTIME,
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_PLAYING,
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_ENABLE_TRACING,
    NANOEM_DOCUMENT_PHYSICS_SIMULATION_MODE_MAX_ENUM
};

/**
 * \defgroup nanoem_document_motion_t Motion
 * @{
 */

/**
* \defgroup nanoem_document_outide_parent_t Outside Parent Object
* @{
*/

/**
 * \brief
 *
 * \param parent
 */
NANOEM_DECL_API const nanoem_document_model_t * APIENTRY
nanoemDocumentOutsideParentGetModelObject(const nanoem_document_outside_parent_t *parent);

/**
 * \brief
 *
 * \param parent
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentOutsideParentGetBoneName(const nanoem_document_outside_parent_t *parent);
/** @} */

/**
* \defgroup nanoem_document_accessory_keyframe_t Accessory Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentBaseKeyframeGetFrameIndex(const nanoem_document_base_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentBaseKeyframeGetPreviousKeyframeIndex(const nanoem_document_base_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentBaseKeyframeGetNextKeyframeIndex(const nanoem_document_base_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentBaseKeyframeGetIndex(const nanoem_document_base_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentBaseKeyframeIsSelected(const nanoem_document_base_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_accessory_keyframe_t Accessory Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetBaseKeyframeObject(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_model_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetParentModelObject(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetParentModelBoneName(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetTranslation(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentAccessoryKeyframeGetOrientation(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentAccessoryKeyframeGetScaleFactor(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentAccessoryKeyframeGetOpacity(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentAccessoryKeyframeIsVisible(const nanoem_document_accessory_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentAccessoryKeyframeIsShadowEnabled(const nanoem_document_accessory_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_model_bone_keyframe_t Bone Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetBaseKeyframeObject(const nanoem_document_model_bone_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetName(const nanoem_document_model_bone_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetTranslation(const nanoem_document_model_bone_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetOrientation(const nanoem_document_model_bone_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 * \param type
 */
NANOEM_DECL_API const nanoem_u8_t * APIENTRY
nanoemDocumentModelBoneKeyframeGetInterpolation(const nanoem_document_model_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t type);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelBoneKeyframeIsPhysicsSimulationDisabled(const nanoem_document_model_bone_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_camera_keyframe_t Camera Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentCameraKeyframeGetBaseKeyframeObject(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_model_t * APIENTRY
nanoemDocumentCameraKeyframeGetParentModelObject(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentCameraKeyframeGetParentModelBoneName(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentCameraKeyframeGetLookAt(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentCameraKeyframeGetAngle(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 * \param type
 */
NANOEM_DECL_API const nanoem_u8_t * APIENTRY
nanoemDocumentCameraKeyframeGetInterpolation(const nanoem_document_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t type);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentCameraKeyframeGetDistance(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentCameraKeyframeGetFov(const nanoem_document_camera_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentCameraKeyframeIsPerspectiveView(const nanoem_document_camera_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_gravity_keyframe_t Gravity Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentGravityKeyframeGetBaseKeyframeObject(const nanoem_document_gravity_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentGravityKeyframeGetDirection(const nanoem_document_gravity_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGravityKeyframeGetAcceleration(const nanoem_document_gravity_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGravityKeyframeGetNoise(const nanoem_document_gravity_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentGravityKeyframeIsNoiseEnabled(const nanoem_document_gravity_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_light_keyframe_t Light Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentLightKeyframeGetBaseKeyframeObject(const nanoem_document_light_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentLightKeyframeGetColor(const nanoem_document_light_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentLightKeyframeGetDirection(const nanoem_document_light_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_model_keyframe_t Model Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelKeyframeGetBaseKeyframeObject(const nanoem_document_model_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_constraint_state_t *const * APIENTRY
nanoemDocumentModelKeyframeGetAllModelConstraintStateObjects(const nanoem_document_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param keyframe
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_outside_parent_t *const * APIENTRY
nanoemDocumentModelKeyframeGetAllOutsideParentObjects(const nanoem_document_model_keyframe_t *keyframe, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelKeyframeIsVisible(const nanoem_document_model_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_model_morph_keyframe_t Morph Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentModelMorphKeyframeGetBaseKeyframeObject(const nanoem_document_model_morph_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelMorphKeyframeGetName(const nanoem_document_model_morph_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentModelMorphKeyframeGetWeight(const nanoem_document_model_morph_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_document_self_shadow_keyframe_t Self Shadow Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API const nanoem_document_base_keyframe_t * APIENTRY
nanoemDocumentSelfShadowKeyframeGetBaseKeyframeObject(const nanoem_document_self_shadow_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentSelfShadowKeyframeGetDistance(const nanoem_document_self_shadow_keyframe_t *keyframe);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentSelfShadowKeyframeGetMode(const nanoem_document_self_shadow_keyframe_t *keyframe);
/** @} */

/** @} */

/**
* \defgroup nanoem_document_model_bone_state_t Model Bone State Object
* @{
*/

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelBoneStateGetName(const nanoem_document_model_bone_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentModelBoneStateGetTranslation(const nanoem_document_model_bone_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentModelBoneStateGetOrientation(const nanoem_document_model_bone_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentModelBoneStateCountSelectedTracks(const nanoem_document_model_bone_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelBoneStateIsDirty(const nanoem_document_model_bone_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelBoneStateIsPhysicsSimulationDisabled(const nanoem_document_model_bone_state_t *state);
/** @} */

/**
* \defgroup nanoem_document_model_constraint_state_t Model Constraint State Object
* @{
*/

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelConstraintStateGetName(const nanoem_document_model_constraint_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelConstraintStateIsEnabled(const nanoem_document_model_constraint_state_t *state);
/** @} */

/**
* \defgroup nanoem_document_model_morph_state_t Model Morph State Object
* @{
*/

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelMorphStateGetName(const nanoem_document_model_morph_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentModelMorphStateGetWeight(const nanoem_document_model_morph_state_t *state);
/** @} */

/**
* \defgroup nanoem_document_outside_parent_state_t Outside Parent State Object
* @{
*/

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentModelOutsideParentStateGetBeginFrameIndex(const nanoem_document_model_outside_parent_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentModelOutsideParentStateGetEndFrameIndex(const nanoem_document_model_outside_parent_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_document_model_t *APIENTRY
nanoemDocumentModelOutsideParentStateGetTargetModelObject(const nanoem_document_model_outside_parent_state_t *state);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelOutsideParentStateGetTargetBoneName(const nanoem_document_model_outside_parent_state_t *state);
/** @} */

/**
* \defgroup nanoem_document_model_t Model Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_bone_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllBoneKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllModelKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_morph_keyframe_t *const *APIENTRY
nanoemDocumentModelGetAllMorphKeyframeObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_unicode_string_t *const *APIENTRY
nanoemDocumentModelGetAllBoneNameObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_unicode_string_t *const *APIENTRY
nanoemDocumentModelGetAllMorphNameObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_bone_state_t *const *APIENTRY
nanoemDocumentModelGetAllBoneStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_constraint_state_t *const *APIENTRY
nanoemDocumentModelGetAllConstraintStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_morph_state_t *const *APIENTRY
nanoemDocumentModelGetAllMorphStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_outside_parent_state_t *const *APIENTRY
nanoemDocumentModelGetAllOutsideParentStateObjects(const nanoem_document_model_t *model, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param model
 * \param language
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetName(const nanoem_document_model_t *model, nanoem_language_type_t language);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetPath(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetSelectedBoneName(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 * \param category
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentModelGetSelectedMorphName(const nanoem_document_model_t *model, nanoem_model_morph_category_t category);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentModelGetEdgeWidth(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentModelGetLastFrameIndex(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentModelGetDrawOrderIndex(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentModelGetTransformOrderIndex(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentModelGetIndex(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelIsBlendEnabled(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentModelIsSelfShadowEnabled(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemDocumentModelCountAllConstraintBones(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemDocumentModelCountAllOutsideParentSubjectBones(const nanoem_document_model_t *model);

/**
 * \brief
 *
 * \param model
 * \param index
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelGetConstraintBoneName(const nanoem_document_model_t *model, nanoem_rsize_t index);

/**
 * \brief
 *
 * \param model
 * \param index
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentModelGetOutsideParentSubjectBoneName(const nanoem_document_model_t *model, nanoem_rsize_t index);
/** @} */

/**
* \defgroup nanoem_document_accessory_t Accessory Object
* @{
*/

/**
 * \brief
 *
 * \param accessory
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_accessory_keyframe_t *const *APIENTRY
nanoemDocumentAccessoryGetAllAccessoryKeyframeObjects(const nanoem_document_accessory_t *accessory, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetName(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetPath(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_document_model_t *APIENTRY
nanoemDocumentAccessoryGetParentModelObject(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_unicode_string_t *APIENTRY
nanoemDocumentAccessoryGetParentModelBoneName(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentAccessoryGetTranslation(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentAccessoryGetOrientation(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentAccessoryGetScaleFactor(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentAccessoryGetOpacity(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentAccessoryGetDrawOrderIndex(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentAccessoryGetIndex(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsVisible(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsShadowEnabled(const nanoem_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentAccessoryIsAddBlendEnabled(const nanoem_document_accessory_t *accessory);
/** @} */

/**
* \defgroup nanoem_document_camera_t Camera Object
* @{
*/

/**
 * \brief
 *
 * \param camera
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_camera_keyframe_t *const *APIENTRY
nanoemDocumentCameraGetAllCameraKeyframeObjects(const nanoem_document_camera_t *camera, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetLookAt(const nanoem_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetAngle(const nanoem_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentCameraGetPosition(const nanoem_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentCameraGetDistance(const nanoem_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentCameraGetFov(const nanoem_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentCameraIsPerspectiveEnabled(const nanoem_document_camera_t *camera);
/** @} */

/**
* \defgroup nanoem_document_gravity_t Gravity Object
* @{
*/

/**
 * \brief
 *
 * \param gravity
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_gravity_keyframe_t *const *APIENTRY
nanoemDocumentGravityGetAllGravityKeyframeObjects(const nanoem_document_gravity_t *gravity, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentGravityGetDirection(const nanoem_document_gravity_t *gravity);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGravityGetAcceleration(const nanoem_document_gravity_t *gravity);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGravityGetNoise(const nanoem_document_gravity_t *gravity);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentGravityIsNoiseEnabled(const nanoem_document_gravity_t *gravity);
/** @} */

/**
* \defgroup nanoem_document_light_t Light Object
* @{
*/

/**
 * \brief
 *
 * \param light
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_light_keyframe_t *const *APIENTRY
nanoemDocumentLightGetAllLightKeyframeObjects(const nanoem_document_light_t *light, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param light
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentLightGetColor(const nanoem_document_light_t *light);

/**
 * \brief
 *
 * \param light
 */
NANOEM_DECL_API const nanoem_f32_t *APIENTRY
nanoemDocumentLightGetDirection(const nanoem_document_light_t *light);
/** @} */

/**
* \defgroup nanoem_document_self_shadow_t Self Shadow Object
* @{
*/

/**
 * \brief
 *
 * \param self_shadow
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_self_shadow_keyframe_t *const *APIENTRY
nanoemDocumentSelfShadowGetAllSelfShadowKeyframeObjects(const nanoem_document_self_shadow_t *self_shadow, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param self_shadow
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentSelfShadowGetDistance(const nanoem_document_self_shadow_t *self_shadow);

/**
 * \brief
 *
 * \param self_shadow
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentSelfShadowIsEnabled(const nanoem_document_self_shadow_t *self_shadow);
/** @} */

/**
 * \defgroup nanoem_document_t Document Object
 * @{
 */

/**
 * \brief
 *
 * \param factory
 * \param status
 */
NANOEM_DECL_API nanoem_document_t *APIENTRY
nanoemDocumentCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param buffer
 * \param status
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentLoadFromBuffer(nanoem_document_t *document, nanoem_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param callback
 */
NANOEM_DECL_API void APIENTRY
nanoemDocumentSetParseModelCallback(nanoem_document_t *document, nanoem_document_parse_model_callback_t callback);

/**
 * \brief
 *
 * \param document
 * \param user_data
 */
NANOEM_DECL_API void APIENTRY
nanoemDocumentSetParseModelCallbackUserData(nanoem_document_t *document, void *user_data);

/**
 * \brief
 *
 * \param document
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_model_t *const *APIENTRY
nanoemDocumentGetAllModelObjects(const nanoem_document_t *document, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param document
 * \param num_objects
 */
NANOEM_DECL_API nanoem_document_accessory_t *const *APIENTRY
nanoemDocumentGetAllAccessoryObjects(const nanoem_document_t *document, nanoem_rsize_t *num_objects);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_document_camera_t *APIENTRY
nanoemDocumentGetCameraObject(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_document_light_t *APIENTRY
nanoemDocumentGetLightObject(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_document_gravity_t *APIENTRY
nanoemDocumentGetGravityObject(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_document_self_shadow_t *APIENTRY
nanoemDocumentGetSelfShadowObject(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetAudioPath(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetBackgroundVideoPath(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_unicode_string_t * APIENTRY
nanoemDocumentGetBackgroundImagePath(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API const nanoem_f32_t * APIENTRY
nanoemDocumentGetEdgeColor(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGetPreferredFPS(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGetBackgroundVideoScaleFactor(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGetBackgroundImageScaleFactor(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_f32_t APIENTRY
nanoemDocumentGetGroundShadowBrightness(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_document_physics_simulation_mode_t APIENTRY
nanoemDocumentGetPhysicsSimulationMode(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentGetCurrentFrameIndex(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentGetCurrentFrameIndexInTextField(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentGetBeginFrameIndex(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_frame_index_t APIENTRY
nanoemDocumentGetEndFrameIndex(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_document_editing_mode_t APIENTRY
nanoemDocumentGetEditingMode(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetViewportWidth(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetViewportHeight(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetTimelineWidth(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetSelectedModelIndex(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetSelectedAccessoryIndex(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetAccessoryIndexAfterModel(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetBackgroundVideoOffsetX(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetBackgroundVideoOffsetY(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetBackgroundImageOffsetX(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API int APIENTRY
nanoemDocumentGetBackgroundImageOffsetY(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsAudioEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsBackgroundVideoEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsBackgroundImageEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsEditingCLAEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsGridAndAxisShown(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsInformationShown(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsGroundShadowShown(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsLoopEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsBeginFrameIndexEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsEndFrameIndexEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsPhysicsGroundEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsTranslucentGroundShadowEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemDocumentIsBlackBackgroundEnabled(const nanoem_document_t *document);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API void APIENTRY
nanoemDocumentDestroy(nanoem_document_t *document);
/** @} */

/** @} */

/**
 * \defgroup nanoem_mutable_document Mutable Document
 * @{
 */

NANOEM_DECL_OPAQUE(nanoem_mutable_document_outside_parent_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_accessory_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_bone_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_camera_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_gravity_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_light_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_morph_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_self_shadow_keyframe_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_bone_state_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_morph_state_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_constraint_state_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_camera_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_light_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_accessory_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_gravity_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_self_shadow_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_model_outside_parent_state_t);
NANOEM_DECL_OPAQUE(nanoem_mutable_document_t);

/**
 * \defgroup nanoem_mutable_document_motion_t Motion
 * @{
 */

/**
* \defgroup nanoem_mutable_document_outide_parent_t Outside Parent Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_outside_parent_t * APIENTRY
nanoemMutableDocumentOutsideParentCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param op
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentOutsideParentSetModelObject(nanoem_mutable_document_outside_parent_t *op, const nanoem_document_model_t *value);

/**
 * \brief
 *
 * \param op
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentOutsideParentSetBoneName(nanoem_mutable_document_outside_parent_t *op, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param op
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentOutsideParentDestroy(nanoem_mutable_document_outside_parent_t *op);
/** @} */

/**
* \defgroup nanoem_mutable_document_accessory_keyframe_t Accessory Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param accessory
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_accessory_keyframe_t * APIENTRY
nanoemMutableDocumentAccessoryKeyframeCreate(nanoem_mutable_document_accessory_t *accessory, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetParentModelObject(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_document_model_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetParentModelBoneName(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetTranslation(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetOrientation(nanoem_mutable_document_accessory_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetScaleFactor(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetOpacity(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetVisible(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeSetShadowEnabled(nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryKeyframeDestroy(nanoem_mutable_document_accessory_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_bone_keyframe_t Bone Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param value
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_bone_keyframe_t * APIENTRY
nanoemMutableDocumentModelBoneKeyframeCreate(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetTranslation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetOrientation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param type
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetInterpolation(nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_motion_bone_keyframe_interpolation_type_t type, const nanoem_u8_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneKeyframeSetPhysicsSimulationDisabled(nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneKeyframeDestroy(nanoem_mutable_document_model_bone_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_camera_keyframe_t Camera Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param camera
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_camera_keyframe_t * APIENTRY
nanoemMutableDocumentCameraKeyframeCreate(nanoem_mutable_document_camera_t *camera, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetParentModelObject(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_document_model_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetParentModelBoneName(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetLookAt(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetAngle(nanoem_mutable_document_camera_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param type
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetInterpolation(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_motion_camera_keyframe_interpolation_type_t type, const nanoem_u8_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetDistance(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetFov(nanoem_mutable_document_camera_keyframe_t *keyframe, int value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeSetPerspectiveView(nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraKeyframeDestroy(nanoem_mutable_document_camera_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_gravity_keyframe_t Gravity Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param gravity
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_gravity_keyframe_t * APIENTRY
nanoemMutableDocumentGravityKeyframeCreate(nanoem_mutable_document_gravity_t *gravity, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityKeyframeSetDirection(nanoem_mutable_document_gravity_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityKeyframeSetAcceleration(nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityKeyframeSetNoise(nanoem_mutable_document_gravity_keyframe_t *keyframe, int value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityKeyframeSetNoiseEnabled(nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityKeyframeDestroy(nanoem_mutable_document_gravity_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_light_keyframe_t Light Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param light
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_light_keyframe_t * APIENTRY
nanoemMutableDocumentLightKeyframeCreate(nanoem_mutable_document_light_t *light, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightKeyframeSetColor(nanoem_mutable_document_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightKeyframeSetDirection(nanoem_mutable_document_light_keyframe_t *keyframe, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightKeyframeDestroy(nanoem_mutable_document_light_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_keyframe_t Model Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_keyframe_t * APIENTRY
nanoemMutableDocumentModelKeyframeCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param reference
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_keyframe_t * APIENTRY
nanoemMutableDocumentModelKeyframeCreateAsReference(nanoem_mutable_document_model_t *model, nanoem_document_model_keyframe_t *reference, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param name
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelKeyframeSetConstraintEnabled(nanoem_mutable_document_model_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_bool_t value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param parent
 * \param index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelKeyframeInsertOutsideParentObject(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_mutable_document_outside_parent_t *parent, int index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param parent
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelKeyframeRemoveOutsideParentObject(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_mutable_document_outside_parent_t *parent, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelKeyframeSetVisible(nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_bool_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelKeyframeDestroy(nanoem_mutable_document_model_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_morph_keyframe_t Morph Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param value
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_morph_keyframe_t * APIENTRY
nanoemMutableDocumentModelMorphKeyframeCreate(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelMorphKeyframeSetWeight(nanoem_mutable_document_model_morph_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelMorphKeyframeDestroy(nanoem_mutable_document_model_morph_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_self_shadow_keyframe_t Self Shadow Keyframe Object
* @{
*/

/**
 * \brief
 *
 * \param self_shadow
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_self_shadow_keyframe_t * APIENTRY
nanoemMutableDocumentSelfShadowKeyframeCreate(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_status_t *status);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeSetDistance(nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_f32_t value);

/**
 * \brief
 *
 * \param keyframe
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeSetMode(nanoem_mutable_document_self_shadow_keyframe_t *keyframe, int value);

/**
 * \brief
 *
 * \param keyframe
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowKeyframeDestroy(nanoem_mutable_document_self_shadow_keyframe_t *keyframe);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_bone_state_t Model Bone State Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_bone_state_t * APIENTRY
nanoemMutableDocumentModelBoneStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneStateSetTranslation(nanoem_mutable_document_model_bone_state_t *state, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneStateSetOrientation(nanoem_mutable_document_model_bone_state_t *state, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneStateSetPhysicsSimulationDisabled(nanoem_mutable_document_model_bone_state_t *state, nanoem_bool_t value);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelBoneStateDestroy(nanoem_mutable_document_model_bone_state_t *state);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_constraint_state_t Model Constraint State Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_constraint_state_t * APIENTRY
nanoemMutableDocumentModelConstraintStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelConstraintStateSetEnabled(nanoem_mutable_document_model_constraint_state_t *state, nanoem_bool_t value);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelConstraintStateDestroy(nanoem_mutable_document_model_constraint_state_t *state);
/** @} */

/** @} */

/**
* \defgroup nanoem_mutable_document_model_morph_state_t Model Morph State Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_morph_state_t * APIENTRY
nanoemMutableDocumentModelMorphStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelMorphStateSetWeight(nanoem_mutable_document_model_morph_state_t *state, nanoem_f32_t value);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelMorphStateDestroy(nanoem_mutable_document_model_morph_state_t *state);
/** @} */

/**
* \defgroup nanoem_mutable_document_outside_parent_state_t Outside Parent State Object
* @{
*/

/**
 * \brief
 *
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_outside_parent_state_t * APIENTRY
nanoemMutableDocumentModelOutsideParentStateCreate(nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetBeginFrameIndex(nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetEndFrameIndex(nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetTargetModelObject(nanoem_mutable_document_model_outside_parent_state_t *state, const nanoem_document_model_t *value);

/**
 * \brief
 *
 * \param state
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelOutsideParentStateSetTargetBoneName(nanoem_mutable_document_model_outside_parent_state_t *state, const nanoem_unicode_string_t *value);

/**
 * \brief
 *
 * \param state
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelOutsideParentStateDestroy(nanoem_mutable_document_model_outside_parent_state_t *state);
/** @} */

/**
* \defgroup nanoem_mutable_document_model_t Model Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_t * APIENTRY
nanoemMutableDocumentModelCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param model
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_model_t * APIENTRY
nanoemMutableDocumentModelCreateAsReference(nanoem_mutable_document_t *document, nanoem_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param name
 * \param status
 */
NANOEM_DECL_API int APIENTRY
nanoemMutableDocumentModelRegisterBone(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param name
 * \param status
 */
NANOEM_DECL_API int APIENTRY
nanoemMutableDocumentModelRegisterMorph(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param name
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddBoneKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param name
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddModelBoneStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param name
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddModelConstraintStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_constraint_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param name
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddModelMorphStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_state_t *state, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddModelKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param name
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddMorphKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_keyframe_t *keyframe, const nanoem_unicode_string_t *name, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param name
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelAddOutsideParentSubjectBone(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *name, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelInsertModelOutsideParentStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_outside_parent_state_t *state, int index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveBoneKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveModelBoneStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_bone_state_t *state, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveModelConstraintStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_constraint_state_t *state, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveModelMorphStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_state_t *state, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveModelKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveMorphKeyframeObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_morph_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param state
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelRemoveModelOutsideParentStateObject(nanoem_mutable_document_model_t *model, nanoem_mutable_document_model_outside_parent_state_t *state, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param language
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetName(nanoem_mutable_document_model_t *model, nanoem_language_type_t language, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetPath(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetSelectedBoneName(nanoem_mutable_document_model_t *model, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param category
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetSelectedMorphName(nanoem_mutable_document_model_t *model, nanoem_model_morph_category_t category, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetEdgeWidth(nanoem_mutable_document_model_t *model, nanoem_f32_t value);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetLastFrameIndex(nanoem_mutable_document_model_t *model, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetDrawOrderIndex(nanoem_mutable_document_model_t *model, int value);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetTransformOrderIndex(nanoem_mutable_document_model_t *model, int value);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetBlendEnabled(nanoem_mutable_document_model_t *model, nanoem_bool_t value);

/**
 * \brief
 *
 * \param model
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelSetSelfShadowEnabled(nanoem_mutable_document_model_t *model, nanoem_bool_t value);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API nanoem_document_model_t * APIENTRY
nanoemMutableDocumentModelGetOrigin(nanoem_mutable_document_model_t *model);

/**
 * \brief
 *
 * \param model
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentModelDestroy(nanoem_mutable_document_model_t *model);
/** @} */

/**
* \defgroup nanoem_mutable_document_accessory_t Accessory Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param accessory
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryCreateAsReference(nanoem_mutable_document_t *document, nanoem_document_accessory_t *accessory, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryAddAccessoryKeyframeObject(nanoem_mutable_document_accessory_t *accessory, nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryRemoveAccessoryKeyframeObject(nanoem_mutable_document_accessory_t *accessory, nanoem_mutable_document_accessory_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetName(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetPath(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetParentModelObject(nanoem_mutable_document_accessory_t *accessory, const nanoem_document_model_t *value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetParentModelBoneName(nanoem_mutable_document_accessory_t *accessory, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetTranslation(nanoem_mutable_document_accessory_t *accessory, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetOrientation(nanoem_mutable_document_accessory_t *accessory, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetScaleFactor(nanoem_mutable_document_accessory_t *accessory, nanoem_f32_t value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetOpacity(nanoem_mutable_document_accessory_t *accessory, nanoem_f32_t value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetDrawOrderIndex(nanoem_mutable_document_accessory_t *accessory, int value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetVisible(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetShadowEnabled(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value);

/**
 * \brief
 *
 * \param accessory
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessorySetAddBlendEnabled(nanoem_mutable_document_accessory_t *accessory, nanoem_bool_t value);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API nanoem_document_accessory_t * APIENTRY
nanoemMutableDocumentAccessoryGetOrigin(nanoem_mutable_document_accessory_t *accessory);

/**
 * \brief
 *
 * \param accessory
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentAccessoryDestroy(nanoem_mutable_document_accessory_t *accessory);
/** @} */

/**
* \defgroup nanoem_mutable_document_camera_t Camera Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_camera_t * APIENTRY
nanoemMutableDocumentCameraCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param camera
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraAddCameraKeyframeObject(nanoem_mutable_document_camera_t *camera, nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param camera
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraRemoveCameraKeyframeObject(nanoem_mutable_document_camera_t *camera, nanoem_mutable_document_camera_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetLookAt(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetAngle(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetDistance(nanoem_mutable_document_camera_t *camera, nanoem_f32_t value);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetPosition(nanoem_mutable_document_camera_t *camera, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetFov(nanoem_mutable_document_camera_t *camera, nanoem_f32_t value);

/**
 * \brief
 *
 * \param camera
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraSetPerspectiveEnabled(nanoem_mutable_document_camera_t *camera, nanoem_bool_t value);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API nanoem_document_camera_t * APIENTRY
nanoemMutableDocumentCameraGetOrigin(nanoem_mutable_document_camera_t *camera);

/**
 * \brief
 *
 * \param camera
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentCameraDestroy(nanoem_mutable_document_camera_t *camera);
/** @} */

/**
* \defgroup nanoem_mutable_document_gravity_t Gravity Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_gravity_t * APIENTRY
nanoemMutableDocumentGravityCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param gravity
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityAddGravityKeyframeObject(nanoem_mutable_document_gravity_t *gravity, nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param gravity
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityRemoveGravityKeyframeObject(nanoem_mutable_document_gravity_t *gravity, nanoem_mutable_document_gravity_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param gravity
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravitySetDirection(nanoem_mutable_document_gravity_t *gravity, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param gravity
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravitySetAcceleration(nanoem_mutable_document_gravity_t *gravity, nanoem_f32_t value);

/**
 * \brief
 *
 * \param gravity
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravitySetNoise(nanoem_mutable_document_gravity_t *gravity, int value);

/**
 * \brief
 *
 * \param gravity
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravitySetNoiseEnabled(nanoem_mutable_document_gravity_t *gravity, nanoem_bool_t value);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API nanoem_document_gravity_t * APIENTRY
nanoemMutableDocumentGravityGetOrigin(nanoem_mutable_document_gravity_t *gravity);

/**
 * \brief
 *
 * \param gravity
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentGravityDestroy(nanoem_mutable_document_gravity_t *gravity);
/** @} */

/**
* \defgroup nanoem_mutable_document_light_t Light Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_light_t * APIENTRY
nanoemMutableDocumentLightCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param light
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightAddLightKeyframeObject(nanoem_mutable_document_light_t *light, nanoem_mutable_document_light_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param light
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightRemoveLightKeyframeObject(nanoem_mutable_document_light_t *light, nanoem_mutable_document_light_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param light
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightSetColor(nanoem_mutable_document_light_t *light, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param light
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightSetDirection(nanoem_mutable_document_light_t *light, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param light
 */
NANOEM_DECL_API nanoem_document_light_t * APIENTRY
nanoemMutableDocumentLightGetOrigin(nanoem_mutable_document_light_t *light);

/**
 * \brief
 *
 * \param light
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentLightDestroy(nanoem_mutable_document_light_t *light);
/** @} */

/**
* \defgroup nanoem_mutable_document_self_shadow_t Self Shadow Object
* @{
*/

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_self_shadow_t * APIENTRY
nanoemMutableDocumentSelfShadowCreate(nanoem_mutable_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param self_shadow
 * \param keyframe
 * \param frame_index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowAddSelfShadowKeyframeObject(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_frame_index_t frame_index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param self_shadow
 * \param keyframe
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowRemoveSelfShadowKeyframeObject(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_mutable_document_self_shadow_keyframe_t *keyframe, nanoem_status_t *status);

/**
 * \brief
 *
 * \param self_shadow
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowSetDistance(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_f32_t value);

/**
 * \brief
 *
 * \param self_shadow
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowSetEnabled(nanoem_mutable_document_self_shadow_t *self_shadow, nanoem_bool_t value);

/**
 * \brief
 *
 * \param self_shadow
 */
NANOEM_DECL_API nanoem_document_self_shadow_t * APIENTRY
nanoemMutableDocumentSelfShadowGetOrigin(nanoem_mutable_document_self_shadow_t *self_shadow);

/**
 * \brief
 *
 * \param self_shadow
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSelfShadowDestroy(nanoem_mutable_document_self_shadow_t *self_shadow);
/** @} */

/**
 * \defgroup nanoem_mutable_document_t Document Object
 * @{
 */

/**
 * \brief
 *
 * \param factory
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_t *APIENTRY
nanoemMutableDocumentCreate(nanoem_unicode_string_factory_t *factory, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param status
 */
NANOEM_DECL_API nanoem_mutable_document_t *APIENTRY
nanoemMutableDocumentCreateAsReference(nanoem_document_t *document, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param accessory
 * \param index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentInsertAccessoryObject(nanoem_mutable_document_t *document, nanoem_mutable_document_accessory_t *accessory, int index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param model
 * \param index
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentInsertModelObject(nanoem_mutable_document_t *document, nanoem_mutable_document_model_t *model, int index, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param accessory
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentRemoveAccessoryObject(nanoem_mutable_document_t *document, nanoem_mutable_document_accessory_t *accessory, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param model
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentRemoveModelObject(nanoem_mutable_document_t *document, nanoem_mutable_document_model_t *model, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetCameraObject(nanoem_mutable_document_t *document, nanoem_mutable_document_camera_t *value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetGravityObject(nanoem_mutable_document_t *document, nanoem_mutable_document_gravity_t *value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetLightObject(nanoem_mutable_document_t *document, nanoem_mutable_document_light_t *value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetSelfShadowObject(nanoem_mutable_document_t *document, nanoem_mutable_document_self_shadow_t *value);

/**
 * \brief
 *
 * \param document
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetAudioPath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundVideoPath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param value
 * \param status
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundImagePath(nanoem_mutable_document_t *document, const nanoem_unicode_string_t *value, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetEdgeColor(nanoem_mutable_document_t *document, const nanoem_f32_t *value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetPreferredFPS(nanoem_mutable_document_t *document, nanoem_f32_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundVideoScaleFactor(nanoem_mutable_document_t *document, nanoem_f32_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundImageScaleFactor(nanoem_mutable_document_t *document, nanoem_f32_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetPhysicsSimulationMode(nanoem_mutable_document_t *document, nanoem_document_physics_simulation_mode_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetCurrentFrameIndex(nanoem_mutable_document_t *document, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetCurrentFrameIndexInTextField(nanoem_mutable_document_t *document, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBeginFrameIndex(nanoem_mutable_document_t *document, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param documentt
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetEndFrameIndex(nanoem_mutable_document_t *documentt, nanoem_frame_index_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetEditingMode(nanoem_mutable_document_t *document, nanoem_document_editing_mode_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetViewportWidth(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetViewportHeight(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetTimelineWidth(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetSelectedModelIndex(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetSelectedAccessoryIndex(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetAccessoryIndexAfterModel(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundVideoOffsetX(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundVideoOffsetY(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundImageOffsetX(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundImageOffsetY(nanoem_mutable_document_t *document, int value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetAudioEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundVideoEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBackgroundImageEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetEditingCLAEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetGridAndAxisShown(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetInformationShown(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetGroundShadowShown(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetLoopEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBeginFrameIndexEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetEndFrameIndexEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetPhysicsGroundEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetTranslucentGroundShadowEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 * \param value
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentSetBlackBackgroundEnabled(nanoem_mutable_document_t *document, nanoem_bool_t value);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API nanoem_document_t *APIENTRY
nanoemMutableDocumentGetOrigin(nanoem_mutable_document_t *document);

/**
 * \brief
 *
 * \param document
 * \param buffer
 * \param status
 */
NANOEM_DECL_API nanoem_bool_t APIENTRY
nanoemMutableDocumentSaveToBuffer(nanoem_mutable_document_t *document, nanoem_mutable_buffer_t *buffer, nanoem_status_t *status);

/**
 * \brief
 *
 * \param document
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableDocumentDestroy(nanoem_mutable_document_t *document);
/** @} */

/** @} */

/** @} */

#endif /* NANOEM_EXT_DOCUMENT_H_ */
