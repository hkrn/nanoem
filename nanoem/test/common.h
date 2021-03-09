/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of nanoem component and it's licensed under MIT license. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_TEST_COMMON_H_
#define NANOEM_TEST_COMMON_H_

/* catch */
#include "catch2/catch.hpp"

/* nanoem core */
#include "nanoem/nanoem.h"
#include "nanoem/nanoem_p.h"

/* nanoem mutable extension */
#include "nanoem/ext/mutable.h"

/* nanoem motion extension */
#include "nanoem/ext/motion.h"

NANOEM_DECL_API nanoem_unicode_string_factory_t * APIENTRY
nanoemUnicodeStringFactoryCreateEXT(nanoem_status_t *status);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryDestroyEXT(nanoem_unicode_string_factory_t *factory);
NANOEM_DECL_API void APIENTRY
nanoemUnicodeStringFactoryToUtf8OnStackEXT(nanoem_unicode_string_factory_t *factory, const nanoem_unicode_string_t *string, nanoem_rsize_t *length, nanoem_u8_t *buffer, nanoem_rsize_t capacity, nanoem_status_t *status);
NANOEM_DECL_API nanoem_rsize_t APIENTRY
nanoemUnicodeStringGetLength(const nanoem_unicode_string_t *string);

namespace Catch {

template <> struct StringMaker<const nanoem_u8_t *> {
    static std::string convert(const nanoem_u8_t *value);
};
template <> struct StringMaker<const nanoem_f32_t *> {
    static std::string convert(const nanoem_f32_t *value);
};
template <> struct StringMaker<const nanoem_motion_outside_parent_t *> {
    static std::string convert(const nanoem_motion_outside_parent_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_accessory_keyframe_t *> {
    static std::string convert(const nanoem_motion_accessory_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_bone_keyframe_t *> {
    static std::string convert(const nanoem_motion_bone_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_camera_keyframe_t *> {
    static std::string convert(const nanoem_motion_camera_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_light_keyframe_t *> {
    static std::string convert(const nanoem_motion_light_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_model_keyframe_t *> {
    static std::string convert(const nanoem_motion_model_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_morph_keyframe_t *> {
    static std::string convert(const nanoem_motion_morph_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_motion_self_shadow_keyframe_t *> {
    static std::string convert(const nanoem_motion_self_shadow_keyframe_t *const &value);
};
template <> struct StringMaker<const nanoem_model_vertex_t *> {
    static std::string convert(const nanoem_model_vertex_t *const &value);
};
template <> struct StringMaker<const nanoem_model_material_t *> {
    static std::string convert(const nanoem_model_material_t *const &value);
};
template <> struct StringMaker<const nanoem_model_bone_t *> {
    static std::string convert(const nanoem_model_bone_t *const &value);
};
template <> struct StringMaker<const nanoem_model_morph_t *> {
    static std::string convert(const nanoem_model_morph_t *const &value);
};
template <> struct StringMaker<const nanoem_model_label_t *> {
    static std::string convert(const nanoem_model_label_t *const &value);
};
template <> struct StringMaker<const nanoem_model_rigid_body_t *> {
    static std::string convert(const nanoem_model_rigid_body_t *const &value);
};
template <> struct StringMaker<const nanoem_model_joint_t *> {
    static std::string convert(const nanoem_model_joint_t *const &value);
};
template <> struct StringMaker<const nanoem_model_texture_t *> {
    static std::string convert(const nanoem_model_texture_t *const &value);
};

} /* namespace Catch */

namespace nanoem {
namespace test {
namespace matcher {

struct EqualsU8Vec4 : Catch::Matchers::Impl::MatcherBase<const nanoem_u8_t *> {
    EqualsU8Vec4(nanoem_u8_t x, nanoem_u8_t y, nanoem_u8_t z, nanoem_u8_t w);
    EqualsU8Vec4(const EqualsU8Vec4 &v);
    bool match(const nanoem_u8_t *const &v) const override;
    std::string describe() const override;
    nanoem_u8_t m_data[4];
};

struct EqualsVec4 : Catch::Matchers::Impl::MatcherBase<const nanoem_f32_t *> {
    EqualsVec4(nanoem_f32_t x, nanoem_f32_t y, nanoem_f32_t z, nanoem_f32_t w);
    EqualsVec4(const EqualsVec4 &v);
    bool match(const nanoem_f32_t *const &v) const override;
    std::string describe() const override;
    float m_data[4];
};

struct EqualsBoneName : Catch::Matchers::Impl::MatcherBase<const nanoem_model_bone_t *> {
    EqualsBoneName(const std::string &name);
    EqualsBoneName(const EqualsBoneName &v);
    bool match(const nanoem_model_bone_t *const &bone) const override;
    std::string describe() const override;
    const std::string m_name;
};

struct EqualsRigidBodyName
    : Catch::Matchers::Impl::MatcherBase<const nanoem_model_rigid_body_t *> {
    EqualsRigidBodyName(const std::string &name);
    EqualsRigidBodyName(const EqualsRigidBodyName &v);
    bool match(const nanoem_model_rigid_body_t *const &body) const override;
    std::string describe() const override;
    const std::string m_name;
};

struct EqualsTexturePath : Catch::Matchers::Impl::MatcherBase<const nanoem_model_texture_t *> {
    EqualsTexturePath(const std::string &name);
    EqualsTexturePath(const EqualsTexturePath &v);
    bool match(const nanoem_model_texture_t *const &texture) const override;
    std::string describe() const override;
    const std::string m_name;
};

} /* namespace matcher */

namespace {

static inline matcher::EqualsU8Vec4
EqualsU8(nanoem_u8_t x, nanoem_u8_t y, nanoem_u8_t z, nanoem_u8_t w)
{
    return matcher::EqualsU8Vec4(x, y, z, w);
}

static inline matcher::EqualsVec4
EqualsOne(nanoem_f32_t x)
{
    return matcher::EqualsVec4(x, x, x, x);
}

static inline matcher::EqualsVec4
Equals(nanoem_f32_t x, nanoem_f32_t y, nanoem_f32_t z, nanoem_f32_t w)
{
    return matcher::EqualsVec4(x, y, z, w);
}

static inline matcher::EqualsVec4
Equals(const nanoem_f32_t *values)
{
    return matcher::EqualsVec4(values[0], values[1], values[2], values[3]);
}

static inline matcher::EqualsBoneName
EqualsBoneName(const std::string &value)
{
    return matcher::EqualsBoneName(value);
}

static inline matcher::EqualsRigidBodyName
EqualsRigidBodyName(const std::string &value)
{
    return matcher::EqualsRigidBodyName(value);
}

static inline matcher::EqualsTexturePath
EqualsTexturePath(const std::string &value)
{
    return matcher::EqualsTexturePath(value);
}

} /* namespace anonymous */

class BaseScope {
public:
    nanoem_unicode_string_t *newString(const char *value);
    std::string describe(const nanoem_unicode_string_t *name);

protected:
    BaseScope();
    virtual ~BaseScope();

    nanoem_unicode_string_factory_t *m_factory = 0;

private:
    std::vector<nanoem_unicode_string_t *> m_strings;
};

class ModelScope : public BaseScope {
public:
    ModelScope();
    ~ModelScope();

    nanoem_mutable_model_t *newModel();
    nanoem_mutable_model_vertex_t *newVertex();
    nanoem_mutable_model_bone_t *newBone();
    nanoem_mutable_model_constraint_t *newConstraint();
    nanoem_mutable_model_material_t *newMaterial();
    nanoem_mutable_model_morph_t *newMorph();
    nanoem_mutable_model_morph_bone_t *newBoneMorph();
    nanoem_mutable_model_morph_flip_t *newFlipMorph();
    nanoem_mutable_model_morph_group_t *newGroupMorph();
    nanoem_mutable_model_morph_impulse_t *newImpulseMorph();
    nanoem_mutable_model_morph_material_t *newMaterialMorph();
    nanoem_mutable_model_morph_uv_t *newUVMorph();
    nanoem_mutable_model_morph_vertex_t *newVertexMorph();
    nanoem_mutable_model_label_t *newLabel();
    nanoem_mutable_model_label_item_t *newLabelItem(nanoem_model_bone_t *bone);
    nanoem_mutable_model_label_item_t *newLabelItem(nanoem_model_morph_t *morph);
    nanoem_mutable_model_rigid_body_t *newRigidBody();
    nanoem_mutable_model_joint_t *newJoint();
    nanoem_mutable_model_texture_t *newTexture();

    nanoem_mutable_model_vertex_t *appendedVertex();
    nanoem_mutable_model_material_t *appendedMaterial(const char *name = nullptr);
    nanoem_mutable_model_bone_t *appendedBone(const char *name = nullptr);
    nanoem_mutable_model_constraint_t *appendedConstraint();
    nanoem_mutable_model_morph_t *appendedMorph(const char *name = nullptr);
    nanoem_mutable_model_label_t *appendedLabel(const char *name = nullptr);
    nanoem_mutable_model_rigid_body_t *appendedRigidBody(const char *name = nullptr);
    nanoem_mutable_model_joint_t *appendedJoint(const char *name = nullptr);
    nanoem_mutable_model_texture_t *appendedTexture(const char *name = nullptr);
    const nanoem_model_vertex_t *copyVertex(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_material_t *copyMaterial(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_bone_t *copyBone(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_constraint_t *copyConstraint(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMD_1_0, size_t offset = 0);
    const nanoem_model_morph_t *copyMorph(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_label_t *copyLabel(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_rigid_body_t *copyRigidBody(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    const nanoem_model_joint_t *copyJoint(
        nanoem_model_format_type_t format = NANOEM_MODEL_FORMAT_TYPE_PMX_2_0, size_t offset = 0);
    void copy(nanoem_model_format_type_t format);

    nanoem_mutable_model_t *currentModel();
    nanoem_model_t *origin();
    nanoem_model_t *reference();

private:
    std::vector<nanoem_mutable_model_t *> m_models;
    std::vector<nanoem_mutable_model_vertex_t *> m_vertices;
    std::vector<nanoem_mutable_model_bone_t *> m_bones;
    std::vector<nanoem_mutable_model_constraint_t *> m_constraints;
    std::vector<nanoem_mutable_model_material_t *> m_materials;
    std::vector<nanoem_mutable_model_morph_t *> m_morphs;
    std::vector<nanoem_mutable_model_morph_bone_t *> m_morphBones;
    std::vector<nanoem_mutable_model_morph_flip_t *> m_morphFlips;
    std::vector<nanoem_mutable_model_morph_group_t *> m_morphGroups;
    std::vector<nanoem_mutable_model_morph_impulse_t *> m_morphImpulses;
    std::vector<nanoem_mutable_model_morph_material_t *> m_morphMaterials;
    std::vector<nanoem_mutable_model_morph_uv_t *> m_morphUVs;
    std::vector<nanoem_mutable_model_morph_vertex_t *> m_morphVertices;
    std::vector<nanoem_mutable_model_label_t *> m_labels;
    std::vector<nanoem_mutable_model_label_item_t *> m_labelItems;
    std::vector<nanoem_mutable_model_rigid_body_t *> m_rigidBodies;
    std::vector<nanoem_mutable_model_joint_t *> m_joints;
    std::vector<nanoem_mutable_model_texture_t *> m_textures;
    nanoem_model_t *m_copy = 0;
};

class MotionScope : public BaseScope {
public:
    MotionScope();
    ~MotionScope();

    nanoem_buffer_t *newBuffer(nanoem_mutable_buffer_t *buffer);
    nanoem_mutable_buffer_t *newBuffer();
    nanoem_mutable_motion_t *newMotion();
    nanoem_mutable_motion_accessory_keyframe_t *newAccessoryKeyframe();
    nanoem_mutable_motion_bone_keyframe_t *newBoneKeyframe();
    nanoem_mutable_motion_camera_keyframe_t *newCameraKeyframe();
    nanoem_mutable_motion_light_keyframe_t *newLightKeyframe();
    nanoem_mutable_motion_model_keyframe_t *newModelKeyframe();
    nanoem_mutable_motion_morph_keyframe_t *newMorphKeyframe();
    nanoem_mutable_motion_self_shadow_keyframe_t *newSelfShadowKeyframe();
    nanoem_motion_t *origin();

private:
    std::vector<nanoem_mutable_motion_t *> m_motions;
    std::vector<nanoem_mutable_buffer_t *> m_buffers;
    std::vector<nanoem_buffer_t *> m_immutableBuffers;
    std::vector<nanoem_mutable_motion_accessory_keyframe_t *> m_accessoryKeyframes;
    std::vector<nanoem_mutable_motion_bone_keyframe_t *> m_boneKeyframes;
    std::vector<nanoem_mutable_motion_camera_keyframe_t *> m_cameraKeyframes;
    std::vector<nanoem_mutable_motion_light_keyframe_t *> m_lightKeyframes;
    std::vector<nanoem_mutable_motion_model_keyframe_t *> m_modelKeyframes;
    std::vector<nanoem_mutable_motion_morph_keyframe_t *> m_morphKeyframes;
    std::vector<nanoem_mutable_motion_self_shadow_keyframe_t *> m_selfShadowKeyframes;
    nanoem_motion_t *m_copy = 0;
};

} /* namespace test */
} /* namespace nanoem */

#endif /* NANOEM_TEST_COMMON_H_ */
