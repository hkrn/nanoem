/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BASEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BASEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_

#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct BaseKeyframeInterpolationCurveGraphDialog : BaseNonModalDialogWindow {
    static const nanoem_f32_t kCircleRadius;
    static const nanoem_u8_t kInterpolationMaxFloatValue;
    static const int kGridCount;

    BaseKeyframeInterpolationCurveGraphDialog(Project *project, BaseApplicationService *applicationPtr);

    void drawEmptyGraph();
    void drawSingleKeyframeGraph(nanoem_f32_t interval, nanoem_f32_t coef, Vector4 &controlPoint);

    void drawBaseGrid(ImDrawList *drawList);
    void drawBezierCurve(ImDrawList *drawList, const Vector4 &controlPoint);
    void drawBezierCurve(
        ImDrawList *drawList, const Vector4 &controlPoint, ImVec2 &controlPointX, ImVec2 &controlPointY);
    void drawXCircle(ImDrawList *drawList, const ImVec2 rectFrom, const ImVec2 avail, const ImVec2 &controlPointX,
        nanoem_f32_t circleRadius, Vector4 &controlPoint);
    void drawYCircle(ImDrawList *drawList, const ImVec2 rectFrom, const ImVec2 avail, const ImVec2 &controlPointY,
        nanoem_f32_t circleRadius, Vector4 &controlPoint);

    nanoem_f32_t m_devicePixelRatio;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASEKEYFRAMEINTERPOLATIONCURVEGRAPHDIALOG_H_ */
