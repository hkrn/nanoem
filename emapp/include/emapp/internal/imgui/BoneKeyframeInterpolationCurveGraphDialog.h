/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BONEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BONEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_

#include "emapp/internal/imgui/BaseKeyframeInterpolationCurveGraphDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct BoneKeyframeInterpolationCurveGraphDialog : BaseKeyframeInterpolationCurveGraphDialog {
    static const char *const kIdentifier;

    BoneKeyframeInterpolationCurveGraphDialog(Model *model, Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Model *m_activeModel;
    nanoem_motion_bone_keyframe_interpolation_type_t m_type;
    Vector4U8 m_controlPoint;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BONEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_ */
