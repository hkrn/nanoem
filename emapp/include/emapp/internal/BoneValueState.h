/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_INTERNAL_BONEVALUESTATE
#define NANOEM_EMAPP_INTERNAL_BONEVALUESTATE

#include "emapp/IVectorValueState.h"

#include "emapp/PhysicsEngine.h"
#include "emapp/model/BindPose.h"

namespace nanoem {

class Model;
class Project;

namespace internal {

class BaseBoneVectorValueState : public IVectorValueState {
public:
    BaseBoneVectorValueState(const nanoem_model_bone_t *bone, Model *model, Project *project);
    ~BaseBoneVectorValueState();

    void update();
    void commit();

protected:
    const nanoem_model_bone_t *m_bone;
    Project *m_project;
    Model *m_model;
    model::BindPose m_lastBindPose;
};

class BoneTranslationValueState NANOEM_DECL_SEALED : public BaseBoneVectorValueState {
public:
    BoneTranslationValueState(const nanoem_model_bone_t *bone, Model *model, Project *project);
    ~BoneTranslationValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    Vector3 m_currentTranslation;
    Vector3 m_initialTranslation;
};

class BoneOrientationValueState NANOEM_DECL_SEALED : public BaseBoneVectorValueState {
public:
    BoneOrientationValueState(const nanoem_model_bone_t *bone, Model *model, Project *project);
    ~BoneOrientationValueState();

    nanoem_rsize_t numComponents() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *initialValue() const NANOEM_DECL_OVERRIDE;
    const nanoem_f32_t *currentValue() const NANOEM_DECL_OVERRIDE;
    void setValue(const nanoem_f32_t *value) NANOEM_DECL_OVERRIDE;
    void rollback() NANOEM_DECL_OVERRIDE;

private:
    Quaternion m_currentOrientation;
    Quaternion m_initialOrientation;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_BONEVALUESTATE */
