/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_BONE_H_
#define NANOEM_EMAPP_MODEL_BONE_H_

#include "emapp/Forward.h"
#include "emapp/model/Constraint.h"

#include "bx/float4x4_t.h"

namespace nanoem {

class Model;
class Motion;

namespace model {

class Bone NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_bone_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_bone_t *, TinySTLAllocator> Set;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, model::Bone::List, TinySTLAllocator> ListTree;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, model::Bone::Set, TinySTLAllocator> SetTree;
    typedef tinystl::unordered_map<const nanoem_model_bone_t *, StringPair, TinySTLAllocator> OutsideParentMap;
    typedef tinystl::vector<nanoem_model_bone_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_bone_t *, TinySTLAllocator> MutableSet;
    static const Vector4U8 kDefaultBezierControlPoint;
    static const Vector4U8 kDefaultAutomaticBezierControlPoint;
    static const nanoem_u8_t kNameRootParentInJapanese[];
    static const nanoem_u8_t kNameCenterInJapanese[];
    static const nanoem_u8_t kNameCenterOfViewportInJapanese[];
    static const nanoem_u8_t kNameCenterOffsetInJapanese[];
    static const nanoem_u8_t kNameLeftInJapanese[];
    static const nanoem_u8_t kNameRightInJapanese[];
    static const nanoem_u8_t kNameDestinationInJapanese[];
    static const nanoem_u8_t kLeftKneeInJapanese[];
    static const nanoem_u8_t kRightKneeInJapanese[];

    typedef tinystl::pair<int, int> IndexPair;
    typedef tinystl::vector<IndexPair, TinySTLAllocator> IndexSet;
    ~Bone() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_bone_t *bone);
    void resetLanguage(
        const nanoem_model_bone_t *bone, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language);
    void resetLocalTransform() NANOEM_DECL_NOEXCEPT;
    void resetUserTransform() NANOEM_DECL_NOEXCEPT;
    void resetMorphTransform() NANOEM_DECL_NOEXCEPT;
    void synchronizeMotion(const Motion *motion, const nanoem_model_bone_t *bone,
        const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_frame_index_t frameIndex, nanoem_f32_t amount);
    void updateLocalOrientation(const nanoem_model_bone_t *bone, const Model *model) NANOEM_DECL_NOEXCEPT;
    void updateLocalTranslation(const nanoem_model_bone_t *bone) NANOEM_DECL_NOEXCEPT;
    void updateLocalMorphTransform(const nanoem_model_morph_bone_t *morph, nanoem_f32_t weight) NANOEM_DECL_NOEXCEPT;
    void updateLocalTransform(const nanoem_model_bone_t *bone, const Vector3 &translation,
        const Quaternion &orientation) NANOEM_DECL_NOEXCEPT;
    void applyAllLocalTransform(const nanoem_model_bone_t *bone, const Model *model);
    void applyOutsideParentTransform(const nanoem_model_bone_t *bone, const Model *model);
    void updateLocalTransform(const nanoem_model_bone_t *bone) NANOEM_DECL_NOEXCEPT;
    void updateSkinningTransform(const nanoem_model_bone_t *bone, const Matrix4x4 value) NANOEM_DECL_NOEXCEPT;
    void updateSkinningTransform(const nanoem_model_bone_t *bone, const bx::float4x4_t *value) NANOEM_DECL_NOEXCEPT;
    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    bool isDirty() const NANOEM_DECL_NOEXCEPT;
    void setDirty(bool value);
    bool isEditingMasked() const NANOEM_DECL_NOEXCEPT;
    void setEditingMasked(bool value);

    static void constrainOrientation(
        const Vector3 &upperLimit, const Vector3 &lowerLimit, Quaternion &orientation) NANOEM_DECL_NOEXCEPT;
    static bool isSelectable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static bool isMovable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static bool isRotateable(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static int index(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_bone_t *bonePtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_bone_t *bonePtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Matrix3x3 localAxes(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static Vector3 origin(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static Vector3 toVector3(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_NOEXCEPT;
    static Quaternion toQuaternion(const nanoem_motion_bone_keyframe_t *keyframe) NANOEM_DECL_NOEXCEPT;
    static Bone *cast(const nanoem_model_bone_t *bonePtr) NANOEM_DECL_NOEXCEPT;
    static Bone *create();

    const bx::float4x4_t worldTransformMatrix() const NANOEM_DECL_NOEXCEPT;
    const bx::float4x4_t localTransformMatrix() const NANOEM_DECL_NOEXCEPT;
    const bx::float4x4_t normalTransformMatrix() const NANOEM_DECL_NOEXCEPT;
    const bx::float4x4_t skinningTransformMatrix() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 worldTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 localTransform() const NANOEM_DECL_NOEXCEPT;
    Matrix4x4 skinningTransform() const NANOEM_DECL_NOEXCEPT;
    Vector3 worldTransformOrigin() const NANOEM_DECL_NOEXCEPT;
    Vector3 localTransformOrigin() const NANOEM_DECL_NOEXCEPT;
    Quaternion localOrientation() const NANOEM_DECL_NOEXCEPT;
    Quaternion localInherentOrientation() const NANOEM_DECL_NOEXCEPT;
    Quaternion localMorphOrientation() const NANOEM_DECL_NOEXCEPT;
    void setLocalMorphOrientation(const Quaternion &value);
    Quaternion localUserOrientation() const NANOEM_DECL_NOEXCEPT;
    void setLocalUserOrientation(const Quaternion &value);
    Quaternion constraintJointOrientation() const NANOEM_DECL_NOEXCEPT;
    void setConstraintJointOrientation(const Quaternion &value);
    Vector3 localTranslation() const NANOEM_DECL_NOEXCEPT;
    Vector3 localInherentTranslation() const NANOEM_DECL_NOEXCEPT;
    Vector3 localMorphTranslation() const NANOEM_DECL_NOEXCEPT;
    void setLocalMorphTranslation(const Vector3 &value);
    Vector3 localUserTranslation() const NANOEM_DECL_NOEXCEPT;
    void setLocalUserTranslation(const Vector3 &value);
    Vector4U8 bezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT;
    void setBezierControlPoints(nanoem_motion_bone_keyframe_interpolation_type_t index, const Vector4U8 &value);
    bool isLinearInterpolation(nanoem_motion_bone_keyframe_interpolation_type_t index) const NANOEM_DECL_NOEXCEPT;
    void setLinearInterpolation(nanoem_motion_bone_keyframe_interpolation_type_t index, bool value);

private:
    struct PlaceHolder { };
    BX_ALIGN_DECL_16(struct) Matrices
    {
        bx::float4x4_t m_worldTransform;
        bx::float4x4_t m_localTransform;
        bx::float4x4_t m_normalTransform;
        bx::float4x4_t m_skinningTransform;
    };
    struct FrameTransform {
        static const FrameTransform kInitialFrameTransform;
        FrameTransform();
        ~FrameTransform() NANOEM_DECL_NOEXCEPT;
        Vector3 m_translation;
        Quaternion m_orientation;
        Vector4U8 m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
        bool m_enableLinearInterpolation[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    };
    static void destroy(void *opaque, nanoem_model_object_t *object) NANOEM_DECL_NOEXCEPT;
    static void synchronizeTransform(const Motion *motion, const nanoem_model_bone_t *bone,
        const nanoem_model_rigid_body_t *rigidBodyPtr, nanoem_frame_index_t frameIndex, FrameTransform &transform);
    static void createConstraintUnitAxes(const Vector3 &radians, const Vector3 &lowerLimit, const Vector3 &upperLimit,
        Quaternion &x, Quaternion &y, Quaternion &z) NANOEM_DECL_NOEXCEPT;
    static void constrainOrientation(
        const nanoem_model_constraint_joint_t *joint, Quaternion &orientation) NANOEM_DECL_NOEXCEPT;
    static Vector3 toVector3(const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;
    static Quaternion toQuaternion(const nanoem_f32_t *value) NANOEM_DECL_NOEXCEPT;

    void solveConstraint(const nanoem_model_constraint_t *constraintPtr, int numIterations);
    void solveConstraint(const nanoem_model_bone_t *bone);
    Bone(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    String m_name;
    String m_canonicalName;
    Matrices m_matrices;
    Quaternion m_localOrientation;
    Quaternion m_localInherentOrientation;
    Quaternion m_localMorphOrientation;
    Quaternion m_localUserOrientation;
    Quaternion m_constraintJointOrientation;
    Vector3 m_localTranslation;
    Vector3 m_localInherentTranslation;
    Vector3 m_localMorphTranslation;
    Vector3 m_localUserTranslation;
    Vector4U8 m_bezierControlPoints[NANOEM_MOTION_BONE_KEYFRAME_INTERPOLATION_TYPE_MAX_ENUM];
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_BONE_H_ */
