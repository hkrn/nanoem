/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_MODELEDITINGTRAIT_H_
#define NANOEM_EMAPP_INTERNAL_MODELEDITINGTRAIT_H_

#include "emapp/Forward.h"

#include "emapp/model/Bone.h"
#include "emapp/model/Joint.h"
#include "emapp/model/Label.h"
#include "emapp/model/Material.h"
#include "emapp/model/Morph.h"
#include "emapp/model/RigidBody.h"
#include "emapp/model/Vertex.h"

#include "nanoem/ext/mutable.h"

namespace nanoem {

class Model;

namespace internal {

class OBJLoader;

class EditingModelTrait : private NonCopyable {
public:
    class Base : private NonCopyable {
    protected:
        struct ModelScope {
            ModelScope(Model *model);
            ~ModelScope() NANOEM_DECL_NOEXCEPT;
            nanoem_mutable_model_t *m_value;
        };

        Base(Model *model);
        ~Base() NANOEM_DECL_NOEXCEPT;

        void handleStatus(nanoem_status_t status);

        Model *m_model;
    };
    class Vertex : public Base {
    public:
        Vertex(Model *model);
        ~Vertex() NANOEM_DECL_NOEXCEPT;

        void normalizeAllVertices();

    private:
    };
    class Material : public Base {
    public:
        static model::Material::MutableList sortedAscent(const model::Material::MutableSet &value);
        static model::Material::MutableList sortedDescent(const model::Material::MutableSet &value);

        Material(Model *model);
        ~Material() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::Material::MutableSet &value);
        void moveItemTop(const model::Material::MutableSet &value);
        void moveItemUp(const model::Material::MutableSet &value);
        void moveItemDown(const model::Material::MutableSet &value);
        void moveItemBottom(const model::Material::MutableSet &value);
        void mergeAllDuplicates();

        void createMaterial(const OBJLoader *loader);

    private:
        typedef tinystl::vector<nanoem_u32_t, TinySTLAllocator> VertexIndexList;
        typedef tinystl::pair<const nanoem_u32_t *, nanoem_rsize_t> VertexIndexOffset;
        typedef tinystl::unordered_map<nanoem_model_material_t *, VertexIndexOffset, TinySTLAllocator>
            MaterialVertexIndicesMap;
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::Material::MutableSet &value, int offset);
        void getMaterialVertexIndicesMap(MaterialVertexIndicesMap &mvi);
        void resetVertexIndices(const MaterialVertexIndicesMap &mvi);
    };
    class Bone : public Base {
    public:
        static model::Bone::MutableList sortedAscent(const model::Bone::MutableSet &value);
        static model::Bone::MutableList sortedDescent(const model::Bone::MutableSet &value);

        Bone(Model *model);
        ~Bone() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::Bone::MutableSet &value);
        void moveItemTop(const model::Bone::MutableSet &value);
        void moveItemUp(const model::Bone::MutableSet &value);
        void moveItemDown(const model::Bone::MutableSet &value);
        void moveItemBottom(const model::Bone::MutableSet &value);
        void mergeAllDuplicates();

        void createRootParentBone();
        void createStagingProxyBone(nanoem_model_bone_t *baseBone);
        void createLabel(const model::Bone::MutableSet &value);
        void createConstraintChain(const model::Bone::MutableSet &value);

    private:
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::Bone::MutableSet &value, int offset);
        void setStagingProxyBoneName(nanoem_mutable_model_bone_t *newBone, const nanoem_model_bone_t *bone,
            int newStageIndex, nanoem_language_type_t language);
        void bind(nanoem_mutable_model_bone_t *bonePtr);
        void bind(nanoem_mutable_model_constraint_t *constraintPtr);
    };
    class Morph : public Base {
    public:
        static model::Morph::MutableList sortedAscent(const model::Morph::MutableSet &value);
        static model::Morph::MutableList sortedDescent(const model::Morph::MutableSet &value);

        Morph(Model *model);
        ~Morph() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::Morph::MutableSet &value);
        void moveItemTop(const model::Morph::MutableSet &value);
        void moveItemUp(const model::Morph::MutableSet &value);
        void moveItemDown(const model::Morph::MutableSet &value);
        void moveItemBottom(const model::Morph::MutableSet &value);
        void mergeAllDuplicates();

        void createVertexMorph(const OBJLoader *loader);
        void createGroupMorphFromCurrentPose();
        void createBoneMorphFromCurrentPose();
        void createVertexMorphFromCurrentPose();
        void createLabel(const model::Morph::MutableSet &value);

    private:
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::Morph::MutableSet &value, int offset);
        void bind(nanoem_mutable_model_morph_t *morphPtr);
    };
    class Label : public Base {
    public:
        static model::Label::MutableList sortedAscent(const model::Label::MutableSet &value);
        static model::Label::MutableList sortedDescent(const model::Label::MutableSet &value);

        Label(Model *model);
        ~Label() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::Label::MutableSet &value);
        void moveItemTop(const model::Label::MutableSet &value);
        void moveItemUp(const model::Label::MutableSet &value);
        void moveItemDown(const model::Label::MutableSet &value);
        void moveItemBottom(const model::Label::MutableSet &value);
        void mergeAllDuplicates();

    private:
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::Label::MutableSet &value, int offset);
    };
    class RigidBody : public Base {
    public:
        static model::RigidBody::MutableList sortedAscent(const model::RigidBody::MutableSet &value);
        static model::RigidBody::MutableList sortedDescent(const model::RigidBody::MutableSet &value);

        RigidBody(Model *model);
        ~RigidBody() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::RigidBody::MutableSet &value);
        void moveItemTop(const model::RigidBody::MutableSet &value);
        void moveItemUp(const model::RigidBody::MutableSet &value);
        void moveItemDown(const model::RigidBody::MutableSet &value);
        void moveItemBottom(const model::RigidBody::MutableSet &value);
        void mergeAllDuplicates();

        void createRigidBodyFromBone(const nanoem_model_bone_t *value);

    private:
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::RigidBody::MutableSet &value, int offset);
    };
    class Joint : public Base {
    public:
        static model::Joint::MutableList sortedAscent(const model::Joint::MutableSet &value);
        static model::Joint::MutableList sortedDescent(const model::Joint::MutableSet &value);

        Joint(Model *model);
        ~Joint() NANOEM_DECL_NOEXCEPT;

        void addItem();
        void removeItem(const model::Joint::MutableSet &value);
        void moveItemTop(const model::Joint::MutableSet &value);
        void moveItemUp(const model::Joint::MutableSet &value);
        void moveItemDown(const model::Joint::MutableSet &value);
        void moveItemBottom(const model::Joint::MutableSet &value);
        void mergeAllDuplicates();

        void createJointFromRigidBody(const nanoem_model_rigid_body_t *a, const nanoem_model_rigid_body_t *b);

    private:
        static int compareAscent(const void *left, const void *right);
        static int compareDescent(const void *left, const void *right);
        static int compareCommon(const void *left, const void *right);

        void moveItemAt(const model::Joint::MutableSet &value, int offset);
    };
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_MODELEDITINGTRAIT_H_ */
