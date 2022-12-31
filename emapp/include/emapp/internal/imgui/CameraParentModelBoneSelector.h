/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELBONESELECTOR_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELBONESELECTOR_H_

#include "emapp/model/Bone.h"

namespace nanoem {

class ITranslator;

namespace internal {
namespace imgui {

struct CameraParentModelBoneSelector {
    static bool callback(void *userData, int index, const char **out) NANOEM_DECL_NOEXCEPT;
    CameraParentModelBoneSelector(const Model *model, const ITranslator *translator);
    bool combo(int *modelIndex);
    bool select(int index, const char **out) const NANOEM_DECL_NOEXCEPT;
    int index(const StringPair &pair) const;
    int count() const NANOEM_DECL_NOEXCEPT;
    const nanoem_model_bone_t *resolveBone(int offset) const NANOEM_DECL_NOEXCEPT;
    String resolveName(int offset) const;

    const ITranslator *m_translator;
    const Model *m_model;
    model::Bone::List m_bones;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARENTMODELBONESELECTOR_H_ */
