/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./json.h"

#include "./mutable.h"
#include "./mutable_p.h"

NANOEM_DECL_INLINE static void
nanoemJSONReadVector3(const JSON_Array *a, nanoem_f32_t *value)
{
    if (json_array_get_count(a) == 3) {
        value[0] = (nanoem_f32_t) json_array_get_number(a, 0);
        value[1] = (nanoem_f32_t) json_array_get_number(a, 1);
        value[2] = (nanoem_f32_t) json_array_get_number(a, 2);
    }
    else {
        value[0] = value[1] = value[2] = 0;
    }
}

NANOEM_DECL_INLINE static JSON_Value *
nanoemJSONWriteVector3(const nanoem_f32_t *value)
{
    JSON_Value *v = json_value_init_array();
    JSON_Array *a = json_array(v);
    json_array_append_number(a, value[0]);
    json_array_append_number(a, value[1]);
    json_array_append_number(a, value[2]);
    return v;
}

NANOEM_DECL_INLINE static void
nanoemJSONReadVector4(const JSON_Array *a, nanoem_f32_t *value)
{
    if (json_array_get_count(a) == 4) {
        value[0] = (nanoem_f32_t) json_array_get_number(a, 0);
        value[1] = (nanoem_f32_t) json_array_get_number(a, 1);
        value[2] = (nanoem_f32_t) json_array_get_number(a, 2);
        value[3] = (nanoem_f32_t) json_array_get_number(a, 3);
    }
    else {
        value[0] = value[1] = value[2] = value[3] = 0;
    }
}

NANOEM_DECL_INLINE static JSON_Value *
nanoemJSONWriteVector4(const nanoem_f32_t *value)
{
    JSON_Value *v = json_value_init_array();
    JSON_Array *a = json_array(v);
    json_array_append_number(a, value[0]);
    json_array_append_number(a, value[1]);
    json_array_append_number(a, value[2]);
    json_array_append_number(a, value[3]);
    return v;
}

NANOEM_DECL_INLINE static void
nanoemJSONReadQuaternion(const JSON_Array *a, nanoem_f32_t *value)
{
    if (json_array_get_count(a) == 4) {
        value[0] = (nanoem_f32_t) json_array_get_number(a, 0);
        value[1] = (nanoem_f32_t) json_array_get_number(a, 1);
        value[2] = (nanoem_f32_t) json_array_get_number(a, 2);
        value[3] = (nanoem_f32_t) json_array_get_number(a, 3);
    }
    else {
        value[0] = value[1] = value[2] = 0;
        value[3] = 1;
    }
}

NANOEM_DECL_INLINE static void
nanoemJSONReadInterpolationParameters(const JSON_Array *a, nanoem_u8_t *value)
{
    if (json_array_get_count(a) == 4) {
        value[0] = (nanoem_u8_t) json_array_get_number(a, 0);
        value[1] = (nanoem_u8_t) json_array_get_number(a, 1);
        value[2] = (nanoem_u8_t) json_array_get_number(a, 2);
        value[3] = (nanoem_u8_t) json_array_get_number(a, 3);
    }
    else {
        memcpy(value, __nanoem_default_interpolation, sizeof(__nanoem_default_interpolation));
    }
}

NANOEM_DECL_INLINE static JSON_Value *
nanoemJSONWriteInterpolationParameters(const nanoem_u8_t *value)
{
    JSON_Value *v = json_value_init_array();
    JSON_Array *a = json_array(v);
    json_array_append_number(a, value[0]);
    json_array_append_number(a, value[1]);
    json_array_append_number(a, value[2]);
    json_array_append_number(a, value[3]);
    return v;
}

NANOEM_DECL_INLINE static void
nanoemJSONSetUnicodeString(const nanoem_unicode_string_t *value, const char *key, JSON_Object *ko, nanoem_unicode_string_factory_t *factory)
{
    nanoem_rsize_t length;
    nanoem_status_t status;
    nanoem_u8_t *s = nanoemUnicodeStringFactoryGetByteArray(factory, value, &length, &status);
    json_object_set_string(ko, key, (const char *) s);
    nanoem_free(s);
}

NANOEM_DECL_INLINE static nanoem_unicode_string_t *
nanoemJSONGetUnicodeString(const char *key, const JSON_Object *ko, nanoem_unicode_string_factory_t *factory)
{
    nanoem_status_t status;
    const char *s = json_object_get_string(ko, key);
    nanoem_unicode_string_t *u = nanoemUnicodeStringFactoryCreateString(factory, (const nanoem_u8_t *) s, s ? strlen(s) : 0, &status);
    return u;
}

static nanoem_bool_t
nanoemJSONReadEffectParameters(const JSON_Object *po, nanoem_mutable_motion_effect_parameter_t *parameter, nanoem_unicode_string_factory_t *factory, nanoem_status_t *status)
{
    nanoem_unicode_string_t *s = nanoemJSONGetUnicodeString("name", po, factory);
    nanoem_f32_t fval, vval[4];
    nanoem_bool_t succeeded = nanoem_true;
    const char *type;
    int ival;
    nanoemMutableMotionEffectParameterSetName(parameter, s, status);
    nanoemUnicodeStringFactoryDestroyString(factory, s);
    type = json_object_get_string(po, "type");
    if (strcmp(type, "bool") == 0) {
        ival = json_object_get_boolean(po, "value");
        nanoemMutableMotionEffectParameterSetValue(parameter, &ival);
    }
    else if (strcmp(type, "int") == 0) {
        ival = (int) json_object_get_number(po, "value");
        nanoemMutableMotionEffectParameterSetValue(parameter, &ival);
    }
    else if (strcmp(type, "float") == 0) {
        fval = (nanoem_f32_t) json_object_get_number(po, "value");
        nanoemMutableMotionEffectParameterSetValue(parameter, &fval);
    }
    else if (strcmp(type, "vector4") == 0) {
        nanoemJSONReadVector4(json_object_get_array(po, "value"), vval);
        nanoemMutableMotionEffectParameterSetValue(parameter, vval);
    }
    else {
        succeeded = nanoem_false;
    }
    return succeeded;
}

static void
nanoemJSONWriteEffectParameters(const nanoem_motion_effect_parameter_t *parameter, JSON_Array *ao, nanoem_unicode_string_factory_t *factory)
{
    JSON_Value *pv = json_value_init_object();
    JSON_Object *po = json_object(pv);
    const void *effect_value;
    nanoemJSONSetUnicodeString(nanoemMotionEffectParameterGetName(parameter), "name", po, factory);
    effect_value = nanoemMotionEffectParameterGetValue(parameter);
    switch (nanoemMotionEffectParameterGetType(parameter)) {
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_BOOL:
        json_object_set_boolean(po, "value", *((const int *) effect_value));
        json_object_set_string(po, "type", "bool");
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_INT:
        json_object_set_number(po, "value", *((const int *) effect_value));
        json_object_set_string(po, "type", "int");
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_FLOAT:
        json_object_set_number(po, "value", *((const nanoem_f32_t *) effect_value));
        json_object_set_string(po, "type", "float");
        break;
    case NANOEM_MOTION_EFFECT_PARAMETER_TYPE_VECTOR4:
        json_object_set_value(po, "value", nanoemJSONWriteVector4((const nanoem_f32_t *) effect_value));
        json_object_set_string(po, "type", "vector4");
        break;
    default:
        break;
    }
    json_array_append_value(ao, pv);
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeConvertFromJSON(nanoem_mutable_motion_accessory_keyframe_t *keyframe, const JSON_Value *value)
{
    JSON_Object *ko, *bo, *po;
    JSON_Array *pa;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *s;
    nanoem_motion_accessory_keyframe_t *origin;
    nanoem_mutable_motion_outside_parent_t *op;
    nanoem_mutable_motion_effect_parameter_t *parameter;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_rsize_t num_parameters, i;
    nanoem_f32_t fv[3], qv[4];
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        origin = keyframe->origin;
        factory = origin->base.parent_motion->factory;
        nanoemMutableMotionAccessoryKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        nanoemJSONReadVector3(json_object_get_array(ko, "translation"), fv);
        nanoemMutableMotionAccessoryKeyframeSetTranslation(keyframe, fv);
        nanoemJSONReadQuaternion(json_object_get_array(ko, "orientation"), qv);
        nanoemMutableMotionAccessoryKeyframeSetOrientation(keyframe, qv);
        nanoemMutableMotionAccessoryKeyframeSetOpacity(keyframe, (nanoem_f32_t) json_object_get_number(ko, "opacity"));
        nanoemMutableMotionAccessoryKeyframeSetScaleFactor(keyframe, (nanoem_f32_t) json_object_get_number(ko, "scaleFactor"));
        bo = json_object_get_object(ko, "binding");
        if (bo) {
            op = nanoemMutableMotionOutsideParentCreateFromAccessoryKeyframe(nanoemMutableMotionAccessoryKeyframeGetOriginObject(keyframe), &status);
            s = nanoemJSONGetUnicodeString("object", bo, factory);
            nanoemMutableMotionOutsideParentSetTargetObjectName(op, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            s = nanoemJSONGetUnicodeString("bone", bo, factory);
            nanoemMutableMotionOutsideParentSetTargetBoneName(op, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            nanoemMutableMotionAccessoryKeyframeSetOutsideParent(keyframe, op, &status);
            nanoemMutableMotionOutsideParentDestroy(op);
        }
        pa = json_object_get_array(ko, "parameters");
        num_parameters = (nanoem_rsize_t) json_array_get_count(pa);
        for (i = 0; i < num_parameters; i++) {
            po = json_array_get_object(pa, i);
            parameter = nanoemMutableMotionEffectParameterCreateFromAccessoryKeyframe(origin, &status);
            if (nanoemJSONReadEffectParameters(po, parameter, factory, &status)) {
                nanoemMutableMotionAccessoryKeyframeAddEffectParameter(keyframe, parameter, &status);
            }
            nanoemMutableMotionEffectParameterDestroy(parameter);
        }
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeConvertFromJSON(nanoem_mutable_motion_bone_keyframe_t *keyframe, const JSON_Value *value, nanoem_status_t *status)
{
    JSON_Object *ko;
    JSON_Array *ia;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *s;
    nanoem_f32_t fv[4];
    nanoem_u8_t iv[4];
    nanoem_motion_bone_keyframe_interpolation_type_t i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        factory = keyframe->origin->base.parent_motion->factory;
        nanoemMutableMotionBoneKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        s = nanoemJSONGetUnicodeString("name", ko, factory);
        nanoemMutableMotionBoneKeyframeSetName(keyframe, s, status);
        nanoemUnicodeStringFactoryDestroyString(factory, s);
        nanoemJSONReadVector3(json_object_get_array(ko, "translation"), fv);
        nanoemMutableMotionBoneKeyframeSetTranslation(keyframe, fv);
        nanoemJSONReadQuaternion(json_object_get_array(ko, "orientation"), fv);
        nanoemMutableMotionBoneKeyframeSetOrientation(keyframe, fv);
        ia = json_object_get_array(ko, "interpolation");
        if (json_array_get_count(ia) == NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
            for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                nanoemJSONReadInterpolationParameters(json_array_get_array(ia, i), iv);
                nanoemMutableMotionBoneKeyframeSetInterpolation(keyframe, i, iv);
            }
        }
        else {
            for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                nanoemMutableMotionBoneKeyframeSetInterpolation(keyframe, i, __nanoem_default_interpolation);
            }
        }
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeConvertFromJSON(nanoem_mutable_motion_camera_keyframe_t *keyframe, const JSON_Value *value)
{
    JSON_Object *ko;
    JSON_Array *ia;
    nanoem_f32_t fv[3];
    nanoem_u8_t iv[4];
    nanoem_motion_camera_keyframe_interpolation_type_t i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        nanoemMutableMotionCameraKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        nanoemJSONReadVector3(json_object_get_array(ko, "angle"), fv);
        nanoemMutableMotionCameraKeyframeSetAngle(keyframe, fv);
        nanoemJSONReadVector3(json_object_get_array(ko, "lookAt"), fv);
        nanoemMutableMotionCameraKeyframeSetLookAt(keyframe, fv);
        nanoemMutableMotionCameraKeyframeSetFov(keyframe, (int) json_object_get_number(ko, "fov"));
        nanoemMutableMotionCameraKeyframeSetDistance(keyframe, (nanoem_f32_t) json_object_get_number(ko, "distance"));
        nanoemMutableMotionCameraKeyframeSetPerspectiveView(keyframe, json_object_get_boolean(ko, "distance") > 0);
        ia = json_object_get_array(ko, "interpolation");
        if (json_array_get_count(ia) == NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM) {
            for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                nanoemJSONReadInterpolationParameters(json_array_get_array(ia, i), iv);
                nanoemMutableMotionCameraKeyframeSetInterpolation(keyframe, i, iv);
            }
        }
        else {
            for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
                nanoemMutableMotionCameraKeyframeSetInterpolation(keyframe, i, __nanoem_default_interpolation);
            }
        }
    }
}

void APIENTRY
nanoemMutableMotionLightKeyframeConvertFromJSON(nanoem_mutable_motion_light_keyframe_t *keyframe, const JSON_Value *value)
{
    JSON_Object *ko;
    nanoem_f32_t fv[3];
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        nanoemMutableMotionLightKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        nanoemJSONReadVector3(json_object_get_array(ko, "color"), fv);
        nanoemMutableMotionLightKeyframeSetColor(keyframe, fv);
        nanoemJSONReadVector3(json_object_get_array(ko, "direction"), fv);
        nanoemMutableMotionLightKeyframeSetDirection(keyframe, fv);
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeConvertFromJSON(nanoem_mutable_motion_model_keyframe_t *keyframe, const JSON_Value *value)
{
    JSON_Object *ko, *so, *bo, *po;
    JSON_Array *sa, *ba, *pa;
    nanoem_motion_model_keyframe_t *origin;
    nanoem_mutable_motion_model_keyframe_constraint_state_t *state;
    nanoem_mutable_motion_outside_parent_t *binding;
    nanoem_mutable_motion_effect_parameter_t *parameter;
    nanoem_status_t status;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *s;
    nanoem_f32_t fv[4];
    nanoem_rsize_t num_states, num_bindings, num_parameters, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        origin = nanoemMutableMotionModelKeyframeGetOriginObject(keyframe);
        factory = keyframe->origin->base.parent_motion->factory;
        nanoemMutableMotionModelKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        nanoemMutableMotionModelKeyframeSetVisible(keyframe, json_object_get_boolean(ko, "visible") > 0);
        nanoemJSONReadVector3(json_object_get_array(ko, "edgeColor"), fv);
        nanoemMutableMotionModelKeyframeSetEdgeColor(keyframe, fv);
        nanoemMutableMotionModelKeyframeSetEdgeScaleFactor(keyframe, (nanoem_f32_t) json_object_get_number(ko, "edgeScaleFactor"));
        sa = json_object_get_array(ko, "states");
        num_states = (nanoem_rsize_t) json_array_get_count(sa);
        for (i = 0; i < num_states; i++) {
            so = json_array_get_object(sa, i);
            state = nanoemMutableMotionModelKeyframeConstraintStateCreate(origin, &status);
            s = nanoemJSONGetUnicodeString("name", so, factory);
            nanoemMutableMotionModelKeyframeConstraintStateSetBoneName(state, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            nanoemMutableMotionModelKeyframeConstraintStateSetEnabled(state, json_object_get_boolean(so, "enabled") != 0);
            nanoemMutableMotionModelKeyframeAddConstraintState(keyframe, state, &status);
            nanoemMutableMotionModelKeyframeConstraintStateDestroy(state);
        }
        ba = json_object_get_array(ko, "bindings");
        num_bindings = (nanoem_rsize_t) json_array_get_count(ba);
        for (i = 0; i < num_bindings; i++) {
            bo = json_array_get_object(ba, i);
            binding = nanoemMutableMotionOutsideParentCreateFromModelKeyframe(origin, &status);
            s = nanoemJSONGetUnicodeString("baseBone", bo, factory);
            nanoemMutableMotionOutsideParentSetSubjectBoneName(binding, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            s = nanoemJSONGetUnicodeString("object", bo, factory);
            nanoemMutableMotionOutsideParentSetTargetObjectName(binding, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            s = nanoemJSONGetUnicodeString("bone", bo, factory);
            nanoemMutableMotionOutsideParentSetTargetBoneName(binding, s, &status);
            nanoemUnicodeStringFactoryDestroyString(factory, s);
            nanoemMutableMotionModelKeyframeAddOutsideParent(keyframe, binding, &status);
            nanoemMutableMotionOutsideParentDestroy(binding);
        }
        pa = json_object_get_array(ko, "parameters");
        num_parameters = (nanoem_rsize_t) json_array_get_count(pa);
        for (i = 0; i < num_parameters; i++) {
            po = json_array_get_object(pa, i);
            parameter = nanoemMutableMotionEffectParameterCreateFromModelKeyframe(origin, &status);
            if (nanoemJSONReadEffectParameters(po, parameter, factory, &status)) {
                nanoemMutableMotionModelKeyframeAddEffectParameter(keyframe, parameter, &status);
            }
            nanoemMutableMotionEffectParameterDestroy(parameter);
        }
    }
}

void APIENTRY
nanoemMutableMotionMorphKeyframeConvertFromJSON(nanoem_mutable_motion_morph_keyframe_t *keyframe, const JSON_Value *value, nanoem_status_t *status)
{
    JSON_Object *ko;
    nanoem_unicode_string_factory_t *factory;
    nanoem_unicode_string_t *s;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        factory = keyframe->origin->base.parent_motion->factory;
        nanoemMutableMotionMorphKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        s = nanoemJSONGetUnicodeString("name", ko, factory);
        nanoemMutableMotionMorphKeyframeSetName(keyframe, s, status);
        nanoemUnicodeStringFactoryDestroyString(factory, s);
        nanoemMutableMotionMorphKeyframeSetWeight(keyframe, (nanoem_f32_t) json_object_get_number(ko, "weight"));
    }
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeConvertFromJSON(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, const JSON_Value *value)
{
    JSON_Object *ko;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        ko = json_object(value);
        nanoemMutableMotionSelfShadowKeyframeSetFrameIndex(keyframe, (nanoem_frame_index_t) json_object_get_number(ko, "frameIndex"));
        nanoemMutableMotionSelfShadowKeyframeSetDistance(keyframe, (nanoem_f32_t) json_object_get_number(ko, "distance"));
        nanoemMutableMotionSelfShadowKeyframeSetMode(keyframe, (int) json_object_get_number(ko, "mode"));
    }
}

void APIENTRY
nanoemMutableMotionAccessoryKeyframeConvertToJSON(nanoem_mutable_motion_accessory_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko, *bo;
    JSON_Array *ao;
    JSON_Value *bv, *av;
    nanoem_motion_accessory_keyframe_t *origin;
    const nanoem_motion_outside_parent_t *binding;
    nanoem_motion_effect_parameter_t *const *parameters, *parameter;
    nanoem_unicode_string_factory_t *factory;
    nanoem_rsize_t num_parameters, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        factory = origin->base.parent_motion->factory;
        ko = json_object(value);
        json_object_set_string(ko, "type", "accessory");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(origin)));
        json_object_set_value(ko, "translation", nanoemJSONWriteVector3(nanoemMotionAccessoryKeyframeGetTranslation(origin)));
        json_object_set_value(ko, "orientation", nanoemJSONWriteVector4(nanoemMotionAccessoryKeyframeGetOrientation(origin)));
        json_object_set_number(ko, "opacity", nanoemMotionAccessoryKeyframeGetOpacity(origin));
        json_object_set_number(ko, "scaleFactor", nanoemMotionAccessoryKeyframeGetScaleFactor(origin));
        binding = nanoemMotionAccessoryKeyframeGetOutsideParent(origin);
        if (binding) {
            bv = json_value_init_object();
            bo = json_object(bv);
            nanoemJSONSetUnicodeString(nanoemMotionOutsideParentGetTargetObjectName(binding), "object", bo, factory);
            nanoemJSONSetUnicodeString(nanoemMotionOutsideParentGetTargetBoneName(binding), "bone", bo, factory);
            json_object_set_value(ko, "binding", bv);
        }
        av = json_value_init_array();
        ao = json_array(av);
        parameters = nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(origin, &num_parameters);
        for (i = 0; i < num_parameters; i++) {
            parameter = parameters[i];
            nanoemJSONWriteEffectParameters(parameter, ao, factory);
        }
        json_object_set_value(ko, "parameters", av);
    }
}

void APIENTRY
nanoemMutableMotionBoneKeyframeConvertToJSON(nanoem_mutable_motion_bone_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko;
    JSON_Array *ia;
    JSON_Value *iv;
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_bone_keyframe_t *origin;
    nanoem_motion_bone_keyframe_interpolation_type_t i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        factory = origin->base.parent_motion->factory;
        ko = json_object(value);
        json_object_set_string(ko, "type", "bone");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(origin)));
        nanoemJSONSetUnicodeString(nanoemMotionBoneKeyframeGetName(origin), "name", ko, factory);
        json_object_set_value(ko, "translation", nanoemJSONWriteVector3(nanoemMotionBoneKeyframeGetTranslation(origin)));
        json_object_set_value(ko, "orientation", nanoemJSONWriteVector4(nanoemMotionBoneKeyframeGetOrientation(origin)));
        iv = json_value_init_array();
        ia = json_array(iv);
        for (i = NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            json_array_append_value(ia, nanoemJSONWriteInterpolationParameters(nanoemMotionBoneKeyframeGetInterpolation(origin, i)));
        }
        json_object_set_value(ko, "interpolation", iv);
    }
}

void APIENTRY
nanoemMutableMotionCameraKeyframeConvertToJSON(nanoem_mutable_motion_camera_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko;
    JSON_Value *iv;
    JSON_Array *ia;
    nanoem_motion_camera_keyframe_t *origin;
    nanoem_motion_camera_keyframe_interpolation_type_t i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        ko = json_object(value);
        json_object_set_string(ko, "type", "camera");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(origin)));
        json_object_set_value(ko, "angle", nanoemJSONWriteVector3(nanoemMotionCameraKeyframeGetAngle(origin)));
        json_object_set_value(ko, "lookAt", nanoemJSONWriteVector3(nanoemMotionCameraKeyframeGetLookAt(origin)));
        json_object_set_number(ko, "distance", nanoemMotionCameraKeyframeGetDistance(origin));
        json_object_set_number(ko, "fov", nanoemMotionCameraKeyframeGetFov(origin));
        json_object_set_boolean(ko, "perspectiveView", nanoemMotionCameraKeyframeIsPerspectiveView(origin));
        iv = json_value_init_array();
        ia = json_array(iv);
        for (i = NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_FIRST_ENUM; i < NANOEM_MOTION_CAMERA_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM; i++) {
            json_array_append_value(ia, nanoemJSONWriteInterpolationParameters(nanoemMotionCameraKeyframeGetInterpolation(origin, i)));
        }
        json_object_set_value(ko, "interpolation", iv);
    }
}

void APIENTRY
nanoemMutableMotionLightKeyframeConvertToJSON(nanoem_mutable_motion_light_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko;
    nanoem_motion_light_keyframe_t *origin;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        ko = json_object(value);
        json_object_set_string(ko, "type", "light");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(origin)));
        json_object_set_value(ko, "color", nanoemJSONWriteVector3(nanoemMotionLightKeyframeGetColor(origin)));
        json_object_set_value(ko, "direction", nanoemJSONWriteVector3(nanoemMotionLightKeyframeGetDirection(origin)));
    }
}

void APIENTRY
nanoemMutableMotionModelKeyframeConvertToJSON(nanoem_mutable_motion_model_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko, *so, *bo;
    JSON_Value *av, *sv, *bv;
    JSON_Array *ao;
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_model_keyframe_t *origin;
    nanoem_motion_model_keyframe_constraint_state_t *const *states, *state;
    nanoem_motion_outside_parent_t *const *bindings, *binding;
    nanoem_motion_effect_parameter_t *const *parameters, *parameter;
    nanoem_rsize_t num_states, num_bindings, num_parameters, i;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        factory = origin->base.parent_motion->factory;
        ko = json_object(value);
        json_object_set_string(ko, "type", "model");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(origin)));
        json_object_set_boolean(ko, "visible", nanoemMotionModelKeyframeIsVisible(origin));
        json_object_set_number(ko, "edgeScaleFactor", nanoemMotionModelKeyframeGetEdgeScaleFactor(origin));
        json_object_set_value(ko, "edgeColor", nanoemJSONWriteVector4(nanoemMotionModelKeyframeGetEdgeColor(origin)));
        av = json_value_init_array();
        ao = json_array(av);
        states = nanoemMotionModelKeyframeGetAllConstraintStateObjects(origin, &num_states);
        for (i = 0; i < num_states; i++) {
            state = states[i];
            sv = json_value_init_object();
            so = json_object(sv);
            nanoemJSONSetUnicodeString(nanoemMotionModelKeyframeConstraintStateGetBoneName(state), "name", so, factory);
            json_object_set_boolean(so, "enabled", nanoemMotionModelKeyframeConstraintStateIsEnabled(state));
            json_array_append_value(ao, sv);
        }
        json_object_set_value(ko, "states", av);
        av = json_value_init_array();
        ao = json_array(av);
        bindings = nanoemMotionModelKeyframeGetAllOutsideParentObjects(origin, &num_bindings);
        for (i = 0; i < num_bindings; i++) {
            binding = bindings[i];
            bv = json_value_init_object();
            bo = json_object(bv);
            nanoemJSONSetUnicodeString(nanoemMotionOutsideParentGetSubjectBoneName(binding), "baseBone", bo, factory);
            nanoemJSONSetUnicodeString(nanoemMotionOutsideParentGetTargetObjectName(binding), "object", bo, factory);
            nanoemJSONSetUnicodeString(nanoemMotionOutsideParentGetTargetBoneName(binding), "bone", bo, factory);
            json_array_append_value(ao, bv);
        }
        json_object_set_value(ko, "bindings", av);
        av = json_value_init_array();
        ao = json_array(av);
        parameters = nanoemMotionModelKeyframeGetAllEffectParameterObjects(origin, &num_parameters);
        for (i = 0; i < num_parameters; i++) {
            parameter = parameters[i];
            nanoemJSONWriteEffectParameters(parameter, ao, factory);
        }
        json_object_set_value(ko, "parameters", av);
    }
}

void APIENTRY
nanoemMutableMotionMorphKeyframeConvertToJSON(nanoem_mutable_motion_morph_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko;
    nanoem_unicode_string_factory_t *factory;
    nanoem_motion_morph_keyframe_t *origin;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        factory = origin->base.parent_motion->factory;
        ko = json_object(value);
        json_object_set_string(ko, "type", "morph");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(origin)));
        nanoemJSONSetUnicodeString(nanoemMotionMorphKeyframeGetName(origin), "name", ko, factory);
        json_object_set_number(ko, "weight", nanoemMotionMorphKeyframeGetWeight(origin));
    }
}

void APIENTRY
nanoemMutableMotionSelfShadowKeyframeConvertToJSON(nanoem_mutable_motion_self_shadow_keyframe_t *keyframe, JSON_Value *value)
{
    JSON_Object *ko;
    nanoem_motion_self_shadow_keyframe_t *origin;
    if (nanoem_is_not_null(keyframe) && nanoem_is_not_null(value)) {
        origin = keyframe->origin;
        ko = json_object(value);
        json_object_set_string(ko, "type", "selfShadow");
        json_object_set_number(ko, "frameIndex", nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(origin)));
        json_object_set_number(ko, "distance", nanoemMotionSelfShadowKeyframeGetDistance(origin));
        json_object_set_number(ko, "mode", nanoemMotionSelfShadowKeyframeGetMode(origin));
    }
}
