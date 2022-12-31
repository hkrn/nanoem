/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#include "./common.h"

#include <assert.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/fwd.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <sstream>

#if !defined(NANOEM_ENABLE_ICU) && !defined(NANOEM_ENABLE_MBWC) && !defined(NANOEM_ENABLE_CFSTRING) && !defined(NANOEM_ENABLE_QT)
nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status)
{
    return nanoemUnicodeStringFactoryCreate(status);
}
void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory)
{
    nanoemUnicodeStringFactoryDestroy(factory);
}
void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status)
{
    char *s = reinterpret_cast<char *>(nanoemUnicodeStringFactoryGetByteArray(factory, string, length, status));
    strncpy(reinterpret_cast<char *>(buffer), s, capacity);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, reinterpret_cast<nanoem_u8_t *>(s));
}
nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string)
{
    return string ? strlen(reinterpret_cast<const char *>(string)) : 0;
}
#endif

namespace {

static std::string
toStdString(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *name)
{
    nanoem_rsize_t length = 0;
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_u8_t *bytes = nanoemUnicodeStringFactoryGetByteArray(factory, name, &length, &status);
    std::string s(bytes, bytes + length);
    nanoemUnicodeStringFactoryDestroyByteArray(factory, bytes);
    return s;
}

static std::string
toStdString(const nanoem_unicode_string_t *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreateEXT(&status);
    const std::string &s = toStdString(factory, name);
    nanoemUnicodeStringFactoryDestroyEXT(factory);
    return s;
}

} /* namespace anonymous */

namespace Catch {

std::string
StringMaker<const nanoem_u8_t *>::convert(const nanoem_u8_t *value)
{
    char buffer[256];
    if (value) {
        snprintf(buffer, sizeof(buffer), "vec4(%d, %d, %d, %d)", value[0], value[1], value[2], value[3]);
    }
    else {
        snprintf(buffer, sizeof(buffer), "%s", "(null)");
    }
    return buffer;
}

std::string
StringMaker<const nanoem_f32_t *>::convert(const nanoem_f32_t *value)
{
    char buffer[256];
    if (value) {
        snprintf(buffer, sizeof(buffer), "vec4(%.4f, %.4f, %.4f, %.4f)", value[0], value[1], value[2], value[3]);
    }
    else {
        snprintf(buffer, sizeof(buffer), "%s", "(null)");
    }
    return buffer;
}

std::string
StringMaker<const nanoem_motion_outside_parent_t *>::convert(const nanoem_motion_outside_parent_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_outside_parent_t";
    os << ", subject_bone_name=" << toStdString(nanoemMotionOutsideParentGetSubjectBoneName(value));
    os << ", target_object_name=" << toStdString(nanoemMotionOutsideParentGetTargetObjectName(value));
    os << ", target_bone_name=" << toStdString(nanoemMotionOutsideParentGetTargetBoneName(value));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_accessory_keyframe_t *>::convert(const nanoem_motion_accessory_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_accessory_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionAccessoryKeyframeGetKeyframeObject(value));
    os << ", translation=" << glm::to_string(glm::make_vec3(nanoemMotionAccessoryKeyframeGetTranslation(value)));
    os << ", orientation=" << glm::to_string(glm::make_vec3(nanoemMotionAccessoryKeyframeGetOrientation(value)));
    os << ", scale=" << nanoemMotionAccessoryKeyframeGetScaleFactor(value);
    os << ", opacity=" << nanoemMotionAccessoryKeyframeGetOpacity(value);
    os << ", visible=" << nanoemMotionAccessoryKeyframeIsVisible(value);
    os << ", blend=" << nanoemMotionAccessoryKeyframeIsAddBlendEnabled(value);
    os << ", shadow=" << nanoemMotionAccessoryKeyframeIsShadowEnabled(value);
    os << ", outside_parent=" << nanoemMotionAccessoryKeyframeGetOutsideParent(value);
    nanoem_rsize_t numItems;
    nanoem_motion_effect_parameter_t *const *parameters =
        nanoemMotionAccessoryKeyframeGetAllEffectParameterObjects(value, &numItems);
    os << ", parameters = {";
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const nanoem_motion_effect_parameter_t *parameter = parameters[i];
        os << " [" << i << "] = " << parameter;
    }
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_bone_keyframe_t *>::convert(const nanoem_motion_bone_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_bone_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(value));
    os << ", name=" << toStdString(nanoemMotionBoneKeyframeGetName(value));
    os << ", translation=" << glm::to_string(glm::make_vec3(nanoemMotionBoneKeyframeGetTranslation(value)));
    os << ", orientation=" << glm::to_string(glm::make_vec3(nanoemMotionBoneKeyframeGetOrientation(value)));
    os << ", physics=" << nanoemMotionBoneKeyframeIsPhysicsSimulationEnabled(value);
    os << ", stage=" << nanoemMotionBoneKeyframeGetStageIndex(value);
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_camera_keyframe_t *>::convert(const nanoem_motion_camera_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_camera_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(value));
    os << ", translation=" << glm::to_string(glm::make_vec3(nanoemMotionCameraKeyframeGetLookAt(value)));
    os << ", orientation=" << glm::to_string(glm::make_vec3(nanoemMotionCameraKeyframeGetAngle(value)));
    os << ", fov=" << nanoemMotionCameraKeyframeGetFov(value);
    os << ", distance=" << nanoemMotionCameraKeyframeGetDistance(value);
    os << ", perspective=" << nanoemMotionCameraKeyframeIsPerspectiveView(value);
    os << ", stage=" << nanoemMotionCameraKeyframeGetStageIndex(value);
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_light_keyframe_t *>::convert(const nanoem_motion_light_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_light_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionLightKeyframeGetKeyframeObject(value));
    os << ", translation=" << glm::to_string(glm::make_vec3(nanoemMotionLightKeyframeGetColor(value)));
    os << ", orientation=" << glm::to_string(glm::make_vec3(nanoemMotionLightKeyframeGetDirection(value)));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_model_keyframe_t *>::convert(const nanoem_motion_model_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_model_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionModelKeyframeGetKeyframeObject(value));
    os << ", visible=" << nanoemMotionModelKeyframeIsVisible(value);
    os << ", edge_color=" << glm::to_string(glm::make_vec3(nanoemMotionModelKeyframeGetEdgeColor(value)));
    os << ", edge_scale_factor=" << nanoemMotionModelKeyframeGetEdgeScaleFactor(value);
    os << ", blend=" << nanoemMotionModelKeyframeIsAddBlendEnabled(value);
    os << ", physics=" << nanoemMotionModelKeyframeIsPhysicsSimulationEnabled(value);
    nanoem_rsize_t numItems;
    nanoem_motion_model_keyframe_constraint_state_t *const *states =
        nanoemMotionModelKeyframeGetAllConstraintStateObjects(value, &numItems);
    os << ", states = { ";
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const nanoem_motion_model_keyframe_constraint_state_t *state = states[i];
        os << " [" << i << "] = " << state;
    }
    os << "}, parameters = {";
    nanoem_motion_effect_parameter_t *const *parameters = nanoemMotionModelKeyframeGetAllEffectParameterObjects(value, &numItems);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const nanoem_motion_effect_parameter_t *parameter = parameters[i];
        os << " [" << i << "] = " << parameter;
    }
    os << "}, outside_parents = {";
    nanoem_motion_outside_parent_t *const *parents = nanoemMotionModelKeyframeGetAllOutsideParentObjects(value, &numItems);
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        const nanoem_motion_outside_parent_t *parent = parents[i];
        os << " [" << i << "] = " << parent;
    }
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_morph_keyframe_t *>::convert(const nanoem_motion_morph_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_morph_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionMorphKeyframeGetKeyframeObject(value));
    os << ", name=" << toStdString(nanoemMotionMorphKeyframeGetName(value));
    os << ", weight=" << nanoemMotionMorphKeyframeGetWeight(value);
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_motion_self_shadow_keyframe_t *>::convert(const nanoem_motion_self_shadow_keyframe_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_motion_self_shadow_keyframe_t";
    os << ", frame_index=" << nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionSelfShadowKeyframeGetKeyframeObject(value));
    os << ", distance=" << nanoemMotionSelfShadowKeyframeGetDistance(value);
    os << ", mode=" << nanoemMotionSelfShadowKeyframeGetMode(value);
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_vertex_t *>::convert(const nanoem_model_vertex_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_vertex_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelVertexGetModelObject(value));
    os << ", origin=" << glm::to_string(glm::make_vec3(nanoemModelVertexGetOrigin(value)));
    os << ", normal=" << glm::to_string(glm::make_vec3(nanoemModelVertexGetNormal(value)));
    os << ", texcoord=" << glm::to_string(glm::make_vec2(nanoemModelVertexGetTexCoord(value)));
    os << ", type=" << nanoemModelVertexGetType(value);
    os << ", edge=" << nanoemModelVertexGetEdgeSize(value);
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_material_t *>::convert(const nanoem_model_material_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_material_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelMaterialGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelMaterialGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_bone_t *>::convert(const nanoem_model_bone_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_bone_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelBoneGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelBoneGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << ", origin=" << glm::to_string(glm::make_vec3(nanoemModelBoneGetOrigin(value)));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_morph_t *>::convert(const nanoem_model_morph_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_morph_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelMorphGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelMorphGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << " }";
    return os.str();
}
std::string
StringMaker<const nanoem_model_label_t *>::convert(const nanoem_model_label_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_label_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelLabelGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelLabelGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_rigid_body_t *>::convert(const nanoem_model_rigid_body_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_rigid_body_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelRigidBodyGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelRigidBodyGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_joint_t *>::convert(const nanoem_model_joint_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_joint_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelJointGetModelObject(value));
    os << ", name=" << toStdString(nanoemModelJointGetName(value, NANOEM_LANGUAGE_TYPE_FIRST_ENUM));
    os << " }";
    return os.str();
}

std::string
StringMaker<const nanoem_model_texture_t *>::convert(const nanoem_model_texture_t *const &value)
{
    std::ostringstream os;
    os << "{ class=nanoem_model_texture_t";
    os << ", index=" << nanoemModelObjectGetIndex(nanoemModelTextureGetModelObject(value));
    os << ", path=" << toStdString(nanoemModelTextureGetPath(value));
    os << " }";
    return os.str();
}

} /* namespace Catch */

namespace nanoem {
namespace test {
namespace matcher {

EqualsU8Vec4::EqualsU8Vec4(nanoem_u8_t x, nanoem_u8_t y, nanoem_u8_t z, nanoem_u8_t w)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
    m_data[3] = w;
}

EqualsU8Vec4::EqualsU8Vec4(const EqualsU8Vec4 &v)
{
    memcpy(m_data, v.m_data, sizeof(m_data));
}

bool
EqualsU8Vec4::match(const nanoem_u8_t * const &v) const
{
    return m_data[0] == v[0] && m_data[1] == v[1] && m_data[2] == v[2] && m_data[3] == v[3];
}

std::string
EqualsU8Vec4::describe() const
{
    return std::string("== ") + Catch::StringMaker<const nanoem_u8_t *>::convert(m_data);
}

EqualsVec4::EqualsVec4(nanoem_f32_t x, nanoem_f32_t y, nanoem_f32_t z, nanoem_f32_t w)
{
    m_data[0] = x;
    m_data[1] = y;
    m_data[2] = z;
    m_data[3] = w;
}

EqualsVec4::EqualsVec4(const EqualsVec4 &v)
{
    memcpy(m_data, v.m_data, sizeof(m_data));
}

bool
EqualsVec4::match(const nanoem_f32_t * const &v) const
{
    return m_data[0] == Approx(v[0]) && m_data[1] == Approx(v[1]) && m_data[2] == Approx(v[2]) &&
        m_data[3] == Approx(v[3]);
}

std::string
EqualsVec4::describe() const
{
    return std::string("== ") + Catch::StringMaker<const nanoem_f32_t *>::convert(m_data);
}

EqualsBoneName::EqualsBoneName(const std::string &name)
    : m_name(name)
{
}

EqualsBoneName::EqualsBoneName(const EqualsBoneName &v)
    : m_name(v.m_name)
{
}

bool
EqualsBoneName::match(const nanoem_model_bone_t * const &bone) const
{
    return toStdString(nanoemModelBoneGetName(bone, NANOEM_LANGUAGE_TYPE_FIRST_ENUM)) == m_name;
}

std::string
EqualsBoneName::describe() const
{
    return "equals: \"" + m_name + '"';
}

EqualsRigidBodyName::EqualsRigidBodyName(const std::string &name)
    : m_name(name)
{
}

EqualsRigidBodyName::EqualsRigidBodyName(const EqualsRigidBodyName &v)
    : m_name(v.m_name)
{
}

bool
EqualsRigidBodyName::match(const nanoem_model_rigid_body_t *const &body) const
{
    return toStdString(nanoemModelRigidBodyGetName(body, NANOEM_LANGUAGE_TYPE_FIRST_ENUM)) == m_name;
}

std::string
EqualsRigidBodyName::describe() const
{
    return "equals: \"" + m_name + '"';
}

EqualsTexturePath::EqualsTexturePath(const std::string &name)
    : m_name(name)
{
}

EqualsTexturePath::EqualsTexturePath(const EqualsTexturePath &v)
    : m_name(v.m_name)
{
}

bool
EqualsTexturePath::match(const nanoem_model_texture_t * const &texture) const
{
    return toStdString(nanoemModelTextureGetPath(texture)) == m_name;
}

std::string
EqualsTexturePath::describe() const
{
    return "equals: \"" + m_name + '"';
}

} /* namespace matcher */

BaseScope::BaseScope()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    m_factory = nanoemUnicodeStringFactoryCreateEXT(&status);
}

BaseScope::~BaseScope()
{
    for (auto it : m_strings) {
        nanoemUnicodeStringFactoryDestroyString(m_factory, it);
    }
    nanoemUnicodeStringFactoryDestroyEXT(m_factory);
}

nanoem_unicode_string_t *
BaseScope::newString(const char *value)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_unicode_string_t *s =
        nanoemUnicodeStringFactoryCreateString(m_factory, (const nanoem_u8_t *) value, strlen(value), &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    m_strings.push_back(s);
    return s;
}

std::string
BaseScope::describe(const nanoem_unicode_string_t *name)
{
    return toStdString(m_factory, name);
}

ModelScope::ModelScope()
    : BaseScope()
{
}

ModelScope::~ModelScope()
{
    for (auto it : m_vertices) {
        nanoemMutableModelVertexDestroy(it);
    }
    for (auto it : m_bones) {
        nanoemMutableModelBoneDestroy(it);
    }
    for (auto it : m_constraints) {
        nanoemMutableModelConstraintDestroy(it);
    }
    for (auto it : m_materials) {
        nanoemMutableModelMaterialDestroy(it);
    }
    for (auto it : m_morphs) {
        nanoemMutableModelMorphDestroy(it);
    }
    for (auto it : m_morphBones) {
        nanoemMutableModelMorphBoneDestroy(it);
    }
    for (auto it : m_morphFlips) {
        nanoemMutableModelMorphFlipDestroy(it);
    }
    for (auto it : m_morphGroups) {
        nanoemMutableModelMorphGroupDestroy(it);
    }
    for (auto it : m_morphImpulses) {
        nanoemMutableModelMorphImpulseDestroy(it);
    }
    for (auto it : m_morphMaterials) {
        nanoemMutableModelMorphMaterialDestroy(it);
    }
    for (auto it : m_morphUVs) {
        nanoemMutableModelMorphUVDestroy(it);
    }
    for (auto it : m_morphVertices) {
        nanoemMutableModelMorphVertexDestroy(it);
    }
    for (auto it : m_labels) {
        nanoemMutableModelLabelDestroy(it);
    }
    for (auto it : m_labelItems) {
        nanoemMutableModelLabelItemDestroy(it);
    }
    for (auto it : m_rigidBodies) {
        nanoemMutableModelRigidBodyDestroy(it);
    }
    for (auto it : m_joints) {
        nanoemMutableModelJointDestroy(it);
    }
    for (auto it : m_textures) {
        nanoemMutableModelTextureDestroy(it);
    }
    for (auto it : m_models) {
        nanoemMutableModelDestroy(it);
    }
    nanoemModelDestroy(m_copy);
}

nanoem_mutable_model_t *
ModelScope::newModel()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_t *model = nanoemMutableModelCreate(m_factory, &status);
    m_models.push_back(model);
    return model;
}

nanoem_mutable_model_vertex_t *
ModelScope::newVertex()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_vertex_t *vertex = nanoemMutableModelVertexCreate(origin(), &status);
    m_vertices.push_back(vertex);
    return vertex;
}

nanoem_mutable_model_bone_t *
ModelScope::newBone()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_bone_t *bone = nanoemMutableModelBoneCreate(origin(), &status);
    m_bones.push_back(bone);
    return bone;
}

nanoem_mutable_model_constraint_t *
ModelScope::newConstraint()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_constraint_t *constraint = nanoemMutableModelConstraintCreate(origin(), &status);
    m_constraints.push_back(constraint);
    return constraint;
}

nanoem_mutable_model_material_t *
ModelScope::newMaterial()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_material_t *material = nanoemMutableModelMaterialCreate(origin(), &status);
    m_materials.push_back(material);
    return material;
}

nanoem_mutable_model_morph_t *
ModelScope::newMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_t *morph = nanoemMutableModelMorphCreate(origin(), &status);
    m_morphs.push_back(morph);
    return morph;
}

nanoem_mutable_model_morph_bone_t *
ModelScope::newBoneMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_bone_t *item = nanoemMutableModelMorphBoneCreate(m_morphs.back(), &status);
    m_morphBones.push_back(item);
    return item;
}

nanoem_mutable_model_morph_flip_t *
ModelScope::newFlipMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_flip_t *item = nanoemMutableModelMorphFlipCreate(m_morphs.back(), &status);
    m_morphFlips.push_back(item);
    return item;
}

nanoem_mutable_model_morph_group_t *
ModelScope::newGroupMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_group_t *item = nanoemMutableModelMorphGroupCreate(m_morphs.back(), &status);
    m_morphGroups.push_back(item);
    return item;
}

nanoem_mutable_model_morph_impulse_t *
ModelScope::newImpulseMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_impulse_t *item = nanoemMutableModelMorphImpulseCreate(m_morphs.back(), &status);
    m_morphImpulses.push_back(item);
    return item;
}

nanoem_mutable_model_morph_material_t *
ModelScope::newMaterialMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_material_t *item = nanoemMutableModelMorphMaterialCreate(m_morphs.back(), &status);
    m_morphMaterials.push_back(item);
    return item;
}

nanoem_mutable_model_morph_uv_t *
ModelScope::newUVMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_uv_t *item = nanoemMutableModelMorphUVCreate(m_morphs.back(), &status);
    m_morphUVs.push_back(item);
    return item;
}

nanoem_mutable_model_morph_vertex_t *
ModelScope::newVertexMorph()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_vertex_t *item = nanoemMutableModelMorphVertexCreate(m_morphs.back(), &status);
    m_morphVertices.push_back(item);
    return item;
}

nanoem_mutable_model_label_t *
ModelScope::newLabel()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_label_t *label = nanoemMutableModelLabelCreate(origin(), &status);
    m_labels.push_back(label);
    return label;
}

nanoem_mutable_model_label_item_t *
ModelScope::newLabelItem(nanoem_model_bone_t *bone)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_label_item_t *item =
        nanoemMutableModelLabelItemCreateFromBoneObject(m_labels.back(), bone, &status);
    m_labelItems.push_back(item);
    return item;
}

nanoem_mutable_model_label_item_t *
ModelScope::newLabelItem(nanoem_model_morph_t *morph)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_label_item_t *item =
        nanoemMutableModelLabelItemCreateFromMorphObject(m_labels.back(), morph, &status);
    m_labelItems.push_back(item);
    return item;
}

nanoem_mutable_model_rigid_body_t *
ModelScope::newRigidBody()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_rigid_body_t *rigid_body = nanoemMutableModelRigidBodyCreate(origin(), &status);
    m_rigidBodies.push_back(rigid_body);
    return rigid_body;
}
nanoem_mutable_model_joint_t *
ModelScope::newJoint()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_joint_t *joint = nanoemMutableModelJointCreate(origin(), &status);
    m_joints.push_back(joint);
    return joint;
}

nanoem_mutable_model_texture_t *
ModelScope::newTexture()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_texture_t *texture = nanoemMutableModelTextureCreate(origin(), &status);
    m_textures.push_back(texture);
    return texture;
}

nanoem_mutable_model_vertex_t *
ModelScope::appendedVertex()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_vertex_t *vertex = newVertex();
    nanoemMutableModelInsertVertexObject(m_models.back(), vertex, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return vertex;
}

nanoem_mutable_model_material_t *
ModelScope::appendedMaterial(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_material_t *material = newMaterial();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelMaterialSetName(material, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertMaterialObject(m_models.back(), material, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return material;
}

nanoem_mutable_model_bone_t *
ModelScope::appendedBone(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_bone_t *bone = newBone();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelBoneSetName(bone, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertBoneObject(m_models.back(), bone, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return bone;
}

nanoem_mutable_model_constraint_t *
ModelScope::appendedConstraint()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_constraint_t *constraint = newConstraint();
    nanoemMutableModelInsertConstraintObject(m_models.back(), constraint, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return constraint;
}

nanoem_mutable_model_morph_t *
ModelScope::appendedMorph(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_morph_t *morph = newMorph();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelMorphSetName(morph, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertMorphObject(m_models.back(), morph, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return morph;
}

nanoem_mutable_model_label_t *
ModelScope::appendedLabel(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_label_t *label = newLabel();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelLabelSetName(label, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertLabelObject(m_models.back(), label, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return label;
}

nanoem_mutable_model_rigid_body_t *
ModelScope::appendedRigidBody(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_rigid_body_t *rigid_body = newRigidBody();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelRigidBodySetName(rigid_body, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertRigidBodyObject(m_models.back(), rigid_body, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return rigid_body;
}

nanoem_mutable_model_joint_t *
ModelScope::appendedJoint(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_joint_t *joint = newJoint();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelJointSetName(joint, s, NANOEM_LANGUAGE_TYPE_FIRST_ENUM, &status);
    }
    nanoemMutableModelInsertJointObject(m_models.back(), joint, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return joint;
}

nanoem_mutable_model_texture_t *
ModelScope::appendedTexture(const char *name)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_model_texture_t *texture = newTexture();
    if (name) {
        nanoem_unicode_string_t *s = newString(name);
        nanoemMutableModelTextureSetPath(texture, s, &status);
    }
    nanoemMutableModelInsertTextureObject(m_models.back(), texture, -1, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    return texture;
}

const nanoem_model_vertex_t *
ModelScope::copyVertex(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_vertices.back());
    /* set dummy vertex indices to prevent error of 1 vertex with 0 vertex index */
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_u32_t indices[] = { 1, 1, 1 };
    nanoemMutableModelSetVertexIndices(currentModel(), indices, 3, &status);
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_vertex_t *const *objects = nanoemModelGetAllVertexObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_material_t *
ModelScope::copyMaterial(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_materials.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_material_t *const *objects = nanoemModelGetAllMaterialObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_bone_t *
ModelScope::copyBone(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_bones.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_bone_t *const *objects = nanoemModelGetAllBoneObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_constraint_t *
ModelScope::copyConstraint(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_constraints.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_constraint_t *const *objects = nanoemModelGetAllConstraintObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_morph_t *
ModelScope::copyMorph(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_morphs.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_morph_t *const *objects = nanoemModelGetAllMorphObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_label_t *
ModelScope::copyLabel(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_labels.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_label_t *const *objects = nanoemModelGetAllLabelObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_rigid_body_t *
ModelScope::copyRigidBody(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_rigidBodies.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_rigid_body_t *const *objects = nanoemModelGetAllRigidBodyObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

const nanoem_model_joint_t *
ModelScope::copyJoint(nanoem_model_format_type_t format, size_t offset)
{
    assert(m_joints.back());
    copy(format);
    nanoem_rsize_t num_objects;
    nanoem_model_joint_t *const *objects = nanoemModelGetAllJointObjects(m_copy, &num_objects);
    assert(num_objects > offset);
    return objects[offset];
}

void
ModelScope::copy(nanoem_model_format_type_t format)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *mutable_buffer = nanoemMutableBufferCreate(&status);
    nanoem_mutable_model_t *model = m_models.back();
    nanoemMutableModelSetFormatType(model, format);
    nanoemMutableModelSaveToBuffer(model, mutable_buffer, &status);
    assert(status == NANOEM_STATUS_SUCCESS);
    nanoemModelDestroy(m_copy);
    m_copy = nanoemModelCreate(m_factory, &status);
    nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutable_buffer, &status);
    nanoemModelLoadFromBuffer(m_copy, buffer, &status);
    nanoemMutableBufferDestroy(mutable_buffer);
    nanoemBufferDestroy(buffer);
    assert(status == NANOEM_STATUS_SUCCESS);
}

nanoem_mutable_model_t *
ModelScope::currentModel()
{
    if (m_models.empty()) {
        newModel();
    }
    return m_models.back();
}

nanoem_model_t *
ModelScope::origin()
{
    return nanoemMutableModelGetOriginObject(currentModel());
}

nanoem_model_t *
ModelScope::reference()
{
    return m_copy;
}

MotionScope::MotionScope()
    : BaseScope()
{
}

MotionScope::~MotionScope()
{
    for (auto it : m_immutableBuffers) {
        nanoemBufferDestroy(it);
    }
    for (auto it : m_buffers) {
        nanoemMutableBufferDestroy(it);
    }
    for (auto it : m_motions) {
        nanoemMutableMotionDestroy(it);
    }
    for (auto it : m_accessoryKeyframes) {
        nanoemMutableMotionAccessoryKeyframeDestroy(it);
    }
    for (auto it : m_boneKeyframes) {
        nanoemMutableMotionBoneKeyframeDestroy(it);
    }
    for (auto it : m_cameraKeyframes) {
        nanoemMutableMotionCameraKeyframeDestroy(it);
    }
    for (auto it : m_lightKeyframes) {
        nanoemMutableMotionLightKeyframeDestroy(it);
    }
    for (auto it : m_modelKeyframes) {
        nanoemMutableMotionModelKeyframeDestroy(it);
    }
    for (auto it : m_morphKeyframes) {
        nanoemMutableMotionMorphKeyframeDestroy(it);
    }
    for (auto it : m_selfShadowKeyframes) {
        nanoemMutableMotionSelfShadowKeyframeDestroy(it);
    }
    nanoemMotionDestroy(m_copy);
}

nanoem_buffer_t *
MotionScope::newBuffer(nanoem_mutable_buffer_t *buffer)
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_buffer_t *immutable = nanoemMutableBufferCreateBufferObject(buffer, &status);
    m_immutableBuffers.push_back(immutable);
    return immutable;
}

nanoem_mutable_buffer_t *
MotionScope::newBuffer()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_buffer_t *buffer = nanoemMutableBufferCreate(&status);
    m_buffers.push_back(buffer);
    return buffer;
}

nanoem_mutable_motion_t *
MotionScope::newMotion()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_t *motion = nanoemMutableMotionCreate(m_factory, &status);
    m_motions.push_back(motion);
    return motion;
}

nanoem_mutable_motion_accessory_keyframe_t *
MotionScope::newAccessoryKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_accessory_keyframe_t *keyframe = nanoemMutableMotionAccessoryKeyframeCreate(origin(), &status);
    m_accessoryKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_bone_keyframe_t *
MotionScope::newBoneKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_bone_keyframe_t *keyframe = nanoemMutableMotionBoneKeyframeCreate(origin(), &status);
    m_boneKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_camera_keyframe_t *
MotionScope::newCameraKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_camera_keyframe_t *keyframe = nanoemMutableMotionCameraKeyframeCreate(origin(), &status);
    m_cameraKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_light_keyframe_t *
MotionScope::newLightKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_light_keyframe_t *keyframe = nanoemMutableMotionLightKeyframeCreate(origin(), &status);
    m_lightKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_model_keyframe_t *
MotionScope::newModelKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_model_keyframe_t *keyframe = nanoemMutableMotionModelKeyframeCreate(origin(), &status);
    m_modelKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_morph_keyframe_t *
MotionScope::newMorphKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_morph_keyframe_t *keyframe = nanoemMutableMotionMorphKeyframeCreate(origin(), &status);
    m_morphKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_mutable_motion_self_shadow_keyframe_t *
MotionScope::newSelfShadowKeyframe()
{
    nanoem_status_t status = NANOEM_STATUS_SUCCESS;
    nanoem_mutable_motion_self_shadow_keyframe_t *keyframe = nanoemMutableMotionSelfShadowKeyframeCreate(origin(), &status);
    m_selfShadowKeyframes.push_back(keyframe);
    return keyframe;
}

nanoem_motion_t *
MotionScope::origin()
{
    if (m_motions.empty()) {
        newMotion();
    }
    return nanoemMutableMotionGetOriginObject(m_motions.back());
}

} /* namespace test */
} /* namespace nanoem */
