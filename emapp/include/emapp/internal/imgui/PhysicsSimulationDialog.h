/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_PHYSICSSIMULATIONRDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_PHYSICSSIMULATIONRDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct PhysicsEngineDialog : BaseNonModalDialogWindow {
    static const char *const kIdentifier;

    PhysicsEngineDialog(Project *project, BaseApplicationService *applicationPtr);
    bool draw(Project *project);

    Vector3 m_direction;
    nanoem_f32_t m_acceleration;
    nanoem_f32_t m_noise;
    nanoem_f32_t m_timeStepFactor;
    bool m_enableNoise;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_PHYSICSSIMULATIONRDIALOG_H_ */
