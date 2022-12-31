/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARAMETERDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARAMETERDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct CameraParametersDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    CameraParametersDialog(Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);
    void reset(Project *project);

    ICamera *m_camera;
    Vector3 m_lookAt;
    Vector3 m_angle;
    nanoem_f32_t m_distance;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_CAMERAPARAMETERDIALOG_H_ */
