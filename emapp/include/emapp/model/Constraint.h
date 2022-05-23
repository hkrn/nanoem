/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_CONSTRAINT_H_
#define NANOEM_EMAPP_MODEL_CONSTRAINT_H_

#include "emapp/Forward.h"

namespace nanoem {
namespace model {

class Constraint NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_constraint_t *, TinySTLAllocator> List;
    struct Joint {
        Joint();
        ~Joint();
        inline void
        setAxis(const Vector3 &a)
        {
            m_axis = a;
        }
        inline void
        setTransform(const Matrix4x4 &value)
        {
            m_orientation = glm::quat_cast(value);
            m_translation = Vector3(value[3]);
        }
        Quaternion m_orientation;
        Vector3 m_translation;
        Vector3 m_targetDirection;
        Vector3 m_effectorDirection;
        Vector3 m_axis;
        nanoem_f32_t m_angle;
    };
    typedef tinystl::vector<Joint, TinySTLAllocator> JointIteration;
    typedef tinystl::unordered_map<const nanoem_model_constraint_joint_t *, JointIteration, TinySTLAllocator>
        JointIterationResult;

    ~Constraint() NANOEM_DECL_NOEXCEPT;

    static int index(const nanoem_model_constraint_t *constraintPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_constraint_t *constraintPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_constraint_t *constraintPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Constraint *cast(const nanoem_model_constraint_t *constraintPtr) NANOEM_DECL_NOEXCEPT;
    static Constraint *create();
    static bool solveAxisAngle(const Matrix4x4 &transform, const Vector4 &effectorPosition,
        const Vector4 &targetPosition, Joint *result) NANOEM_DECL_NOEXCEPT;
    static bool hasUnitXConstraint(
        const nanoem_model_bone_t *bone, nanoem_unicode_string_factory_t *factory) NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_constraint_t *constraintPtr);
    void resetLanguage(const nanoem_model_constraint_t *constraintPtr, nanoem_unicode_string_factory_t *factory,
        nanoem_language_type_t language);
    void initialize(const nanoem_model_constraint_t *constraintPtr);

    const JointIterationResult *jointIterationResult() const NANOEM_DECL_NOEXCEPT;
    const JointIterationResult *effectorIterationResult() const NANOEM_DECL_NOEXCEPT;
    Joint *jointIterationResult(const nanoem_model_constraint_joint_t *joint, nanoem_rsize_t offset);
    Joint *effectorIterationResult(const nanoem_model_constraint_joint_t *joint, nanoem_rsize_t offset);
    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    bool isEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEnabled(bool value);

private:
    struct PlaceHolder { };
    static void destroy(void *opaque, nanoem_model_object_t *constraint) NANOEM_DECL_NOEXCEPT;
    Constraint(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    JointIterationResult m_jointIterationResult;
    JointIterationResult m_effectorIterationResult;
    String m_name;
    String m_canonicalName;
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_CONSTRAINT_H_ */
