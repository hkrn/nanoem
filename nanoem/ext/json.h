/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EXT_JSON_H_
#define NANOEM_EXT_JSON_H_

#include "../nanoem.h"

#ifdef NANOEM_ENABLE_JSON

#include "./parson/parson.h"
#include "./mutable.h"

/**
 * \defgroup nanoem_mutable_json JSON Serializer
 * @{
 */
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeConvertFromJSON(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeConvertFromJSON(nanoem_mutable_motion_bone_keyframe_t *keyframe, const JSON_Value *value, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeConvertFromJSON(nanoem_mutable_motion_camera_keyframe_t *keyframe, const JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeConvertFromJSON(nanoem_mutable_motion_light_keyframe_t *keyframe, const JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConvertFromJSON(nanoem_mutable_motion_model_keyframe_t *keyframe, const JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeConvertFromJSON(nanoem_mutable_motion_morph_keyframe_t *keyframe, const JSON_Value *value, nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeConvertFromJSON(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, const JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionAccessoryKeyframeConvertToJSON(nanoem_mutable_motion_accessory_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionBoneKeyframeConvertToJSON(nanoem_mutable_motion_bone_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionCameraKeyframeConvertToJSON(nanoem_mutable_motion_camera_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionLightKeyframeConvertToJSON(nanoem_mutable_motion_light_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionModelKeyframeConvertToJSON(nanoem_mutable_motion_model_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionMorphKeyframeConvertToJSON(nanoem_mutable_motion_morph_keyframe_t *keyframe, JSON_Value *value);
NANOEM_DECL_API void APIENTRY
nanoemMutableMotionSelfShadowKeyframeConvertToJSON(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, JSON_Value *value);
/** @} */

#endif /* NANOEM_ENABLE_JSON */

#endif /* NANOEM_EXT_JSON_H_ */
