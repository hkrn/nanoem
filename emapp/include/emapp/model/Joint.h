/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODEL_JOINT_H_
#define NANOEM_EMAPP_MODEL_JOINT_H_

#include "emapp/Forward.h"
#include "emapp/model/RigidBody.h"

struct nanoem_physics_joint_t;

namespace nanoem {
namespace model {

class Joint NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef tinystl::vector<const nanoem_model_joint_t *, TinySTLAllocator> List;
    typedef tinystl::unordered_set<const nanoem_model_joint_t *, TinySTLAllocator> Set;
    typedef tinystl::vector<nanoem_model_joint_t *, TinySTLAllocator> MutableList;
    typedef tinystl::unordered_set<nanoem_model_joint_t *, TinySTLAllocator> MutableSet;

    static int index(const nanoem_model_joint_t *jointPtr) NANOEM_DECL_NOEXCEPT;
    static const char *nameConstString(
        const nanoem_model_joint_t *jointPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static const char *canonicalNameConstString(
        const nanoem_model_joint_t *jointPtr, const char *placeHolder) NANOEM_DECL_NOEXCEPT;
    static Joint *cast(const nanoem_model_joint_t *jointPtr) NANOEM_DECL_NOEXCEPT;
    static Joint *create();
    ~Joint() NANOEM_DECL_NOEXCEPT;

    void bind(nanoem_model_joint_t *joint, PhysicsEngine *engine, const RigidBody::Resolver &resolver);
    void resetLanguage(
        const nanoem_model_joint_t *joint, nanoem_unicode_string_factory_t *factory, nanoem_language_type_t language);
    void destroy() NANOEM_DECL_NOEXCEPT;

    String name() const;
    String canonicalName() const;
    const char *nameConstString() const NANOEM_DECL_NOEXCEPT;
    const char *canonicalNameConstString() const NANOEM_DECL_NOEXCEPT;
    PhysicsEngine *physicsEngine() const NANOEM_DECL_NOEXCEPT;
    nanoem_physics_joint_t *physicsJoint() const NANOEM_DECL_NOEXCEPT;
    void getWorldTransformA(nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    void getWorldTransformB(nanoem_f32_t *value) const NANOEM_DECL_NOEXCEPT;
    bool isEditingMasked() const NANOEM_DECL_NOEXCEPT;
    void setEditingMasked(bool value);

    const par_shapes_mesh_s *sharedShapeMesh(const nanoem_model_joint_t *joint);
    void enable();
    void disable();

private:
    struct PlaceHolder { };
    static void destroy(void *opaque, nanoem_model_object_t *joint) NANOEM_DECL_NOEXCEPT;
    Joint(const PlaceHolder &holder) NANOEM_DECL_NOEXCEPT;

    PhysicsEngine *m_physicsEngine;
    nanoem_physics_joint_t *m_physicsJoint;
    par_shapes_mesh_s *m_shape;
    String m_name;
    String m_canonicalName;
    nanoem_u32_t m_states;
};

} /* namespace model */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODEL_JOINT_H_ */
