/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/BoneValueState.h"

#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/command/TransformBoneCommand.h"
#include "emapp/model/Bone.h"

#include "glm/gtc/type_ptr.hpp"

namespace nanoem {
namespace internal {

BaseBoneVectorValueState::BaseBoneVectorValueState(const nanoem_model_bone_t *bone, Model *model, Project *project)
    : m_bone(bone)
    , m_project(project)
    , m_model(model)
{
    nanoem_parameter_assert(m_bone, "must NOT be NULL");
    nanoem_parameter_assert(m_project, "must NOT be NULL");
    nanoem_parameter_assert(m_model, "must NOT be NULL");
    m_model->saveBindPose(m_lastBindPose);
}

BaseBoneVectorValueState::~BaseBoneVectorValueState()
{
    m_bone = 0;
    m_project = 0;
    m_model = 0;
}

void
BaseBoneVectorValueState::update()
{
    m_model->performAllBonesTransform();
}

void
BaseBoneVectorValueState::commit()
{
    model::Bone::List bones;
    bones.push_back(m_bone);
    model::BindPose currentBindPose;
    m_model->saveBindPose(currentBindPose);
    m_model->pushUndo(
        command::TransformBoneCommand::create(m_lastBindPose, currentBindPose, bones, m_model, m_project));
}

BoneTranslationValueState::BoneTranslationValueState(const nanoem_model_bone_t *bone, Model *model, Project *project)
    : BaseBoneVectorValueState(bone, model, project)
    , m_currentTranslation(model::Bone::cast(bone)->localUserTranslation())
    , m_initialTranslation(model::Bone::cast(bone)->localUserTranslation())
{
}

BoneTranslationValueState::~BoneTranslationValueState()
{
}

nanoem_rsize_t
BoneTranslationValueState::numComponents() const
{
    return 3;
}

const nanoem_f32_t *
BoneTranslationValueState::initialValue() const
{
    return glm::value_ptr(m_initialTranslation);
}

const nanoem_f32_t *
BoneTranslationValueState::currentValue() const
{
    return glm::value_ptr(m_currentTranslation);
}

void
BoneTranslationValueState::setValue(const nanoem_f32_t *value)
{
    m_currentTranslation = glm::make_vec3(value);
    model::Bone::cast(m_bone)->setLocalUserTranslation(m_currentTranslation);
    update();
}

void
BoneTranslationValueState::rollback()
{
    model::Bone::cast(m_bone)->setLocalUserTranslation(m_initialTranslation);
    update();
}

BoneOrientationValueState::BoneOrientationValueState(const nanoem_model_bone_t *bone, Model *model, Project *project)
    : BaseBoneVectorValueState(bone, model, project)
    , m_currentOrientation(model::Bone::cast(bone)->localUserOrientation())
    , m_initialOrientation(model::Bone::cast(bone)->localUserOrientation())
{
}

BoneOrientationValueState::~BoneOrientationValueState()
{
}

nanoem_rsize_t
BoneOrientationValueState::numComponents() const
{
    return 4;
}

const nanoem_f32_t *
BoneOrientationValueState::initialValue() const
{
    return glm::value_ptr(m_initialOrientation);
}

const nanoem_f32_t *
BoneOrientationValueState::currentValue() const
{
    return glm::value_ptr(m_currentOrientation);
}

void
BoneOrientationValueState::setValue(const nanoem_f32_t *value)
{
    m_currentOrientation = glm::make_quat(value);
    model::Bone::cast(m_bone)->setLocalUserOrientation(m_currentOrientation);
    update();
}

void
BoneOrientationValueState::rollback()
{
    model::Bone::cast(m_bone)->setLocalUserOrientation(m_initialOrientation);
    update();
}

} /* namespace internal */
} /* namespace nanoem */
