/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/CameraParentModelBoneSelector.h"

#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace internal {
namespace imgui {

bool
CameraParentModelBoneSelector::callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT
{
    const CameraParentModelBoneSelector *self = static_cast<const CameraParentModelBoneSelector *>(userData);
    return self->select(index, out);
}

CameraParentModelBoneSelector::CameraParentModelBoneSelector(const Model *model, const ITranslator *translator)
    : m_translator(translator)
    , m_model(model)
{
    if (model) {
        nanoem_rsize_t numBones;
        nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(model->data(), &numBones);
        if (bones && numBones > 0) {
            m_bones.reserve(numBones);
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                m_bones.push_back(bonePtr);
            }
        }
    }
}

bool
CameraParentModelBoneSelector::combo(int *modelIndex)
{
    return ImGui::Combo("##camera.model.bone", modelIndex, callback, this, count());
}

bool
CameraParentModelBoneSelector::select(int index, const char **out) const NANOEM_DECL_NOEXCEPT
{
    const char *name = nullptr;
    if (index > 0 && index < count()) {
        if (const model::Bone *bone = model::Bone::cast(m_bones[index - 1])) {
            name = bone->nameConstString();
        }
    }
    if (!name) {
        name = m_translator->translate("nanoem.gui.panel.camera.bone.none");
    }
    *out = name;
    return true;
}

int
CameraParentModelBoneSelector::index(const StringPair &pair) const
{
    int value = 0;
    if (!m_bones.empty() && m_model) {
        const String &boneName = pair.second;
        if (!boneName.empty()) {
            value = 1;
            for (model::Bone::List::const_iterator it = m_bones.begin(), end = m_bones.end(); it != end; ++it) {
                const model::Bone *bone = model::Bone::cast(*it);
                if (bone && bone->canonicalName() == boneName) {
                    break;
                }
                value++;
            }
        }
    }
    return value;
}

int
CameraParentModelBoneSelector::count() const NANOEM_DECL_NOEXCEPT
{
    return Inline::saturateInt32(m_bones.size() + 1);
}

const nanoem_model_bone_t *
CameraParentModelBoneSelector::resolveBone(int offset) const NANOEM_DECL_NOEXCEPT
{
    return offset > 0 && offset < count() ? m_bones[offset - 1] : nullptr;
}

String
CameraParentModelBoneSelector::resolveName(int offset) const
{
    const nanoem_model_bone_t *bonePtr = resolveBone(offset);
    return bonePtr ? model::Bone::cast(bonePtr)->canonicalName() : String();
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
