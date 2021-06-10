/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_

#include "emapp/internal/imgui/BaseKeyframeInterpolationCurveGraphDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct CameraKeyframeInterpolationCurveGraphDialog : BaseKeyframeInterpolationCurveGraphDialog {
    static const char *const kIdentifier;

    CameraKeyframeInterpolationCurveGraphDialog(Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    nanoem_motion_camera_keyframe_interpolation_type_t m_type;
    Vector4U8 m_controlPoint;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_ */
