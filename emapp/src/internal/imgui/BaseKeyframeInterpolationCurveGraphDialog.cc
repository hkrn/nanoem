/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BaseKeyframeInterpolationCurveGraphDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

const nanoem_f32_t BaseKeyframeInterpolationCurveGraphDialog::kCircleRadius = 8;
const nanoem_u8_t BaseKeyframeInterpolationCurveGraphDialog::kInterpolationMaxFloatValue = 127;
const int BaseKeyframeInterpolationCurveGraphDialog::kGridCount = 8;

BaseKeyframeInterpolationCurveGraphDialog::BaseKeyframeInterpolationCurveGraphDialog(
    Project *project, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_devicePixelRatio(project->windowDevicePixelRatio())
{
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawEmptyGraph()
{
    const ImVec2 rectFrom(ImGui::GetCursorScreenPos()), avail(256 * m_devicePixelRatio, 256 * m_devicePixelRatio),
        rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(rectFrom, rectTo);
    drawBaseGrid(drawList);
    drawList->AddRect(rectFrom, rectTo, ImGuiWindow::kColorBorder);
    drawList->PopClipRect();
    ImGui::Dummy(avail);
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawSingleKeyframeGraph(
    nanoem_f32_t interval, nanoem_f32_t coef, Vector4 &controlPoint)
{
    const nanoem_f32_t circleRadius = kCircleRadius * m_devicePixelRatio;
    const ImVec2 rectFrom(ImGui::GetCursorScreenPos()), avail(256 * m_devicePixelRatio, 256 * m_devicePixelRatio),
        rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
    ImDrawList *drawList = ImGui::GetWindowDrawList();
    drawList->PushClipRect(rectFrom, rectTo);
    drawBaseGrid(drawList);
    ImVec2 controlPointX, controlPointY;
    drawBezierCurve(drawList, controlPoint, controlPointX, controlPointY);
    const ImVec2 lastCursorPos(ImGui::GetCursorScreenPos());
    drawXCircle(drawList, rectFrom, avail, controlPointX, circleRadius, controlPoint);
    drawYCircle(drawList, rectFrom, avail, controlPointY, circleRadius, controlPoint);
    ImGui::SetCursorScreenPos(lastCursorPos);
    const BezierCurve bc(Vector2(controlPoint.x, controlPoint.y), Vector2(controlPoint.z, controlPoint.w), interval);
    const nanoem_f32_t x2 = rectFrom.x + coef * avail.x;
    const nanoem_f32_t y2 = rectTo.y - bc.value(coef) * avail.y;
    drawList->AddCircleFilled(ImVec2(x2, y2), circleRadius, ImGuiWindow::kColorInterpolationCurveControlPoint);
    drawList->AddRect(rectFrom, rectTo, ImGuiWindow::kColorBorder);
    drawList->PopClipRect();
    ImGui::Dummy(avail);
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawBaseGrid(ImDrawList *drawList)
{
    const ImVec2 &rectFrom = ImGui::GetCursorScreenPos(),
                 &avail = ImVec2(256 * m_devicePixelRatio, 256 * m_devicePixelRatio);
    ImVec2 rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
    drawList->AddRectFilled(rectFrom, rectTo, ImGuiWindow::kColorFrameBg);
    for (int i = 0; i < kGridCount; i++) {
        const nanoem_f32_t y = rectFrom.y + (avail.y / kGridCount) * i;
        const ImVec2 x0(rectFrom.x, y), x1(rectFrom.x + avail.x, y);
        drawList->AddLine(x0, x1, ImGuiWindow::kColorBorder);
    }
    for (int j = 0; j < kGridCount; j++) {
        const nanoem_f32_t x = rectFrom.x + (avail.x / kGridCount) * j;
        const ImVec2 y0(x, rectFrom.y), y1(x, rectFrom.y + avail.y);
        drawList->AddLine(y0, y1, ImGuiWindow::kColorBorder);
    }
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawBezierCurve(ImDrawList *drawList, const Vector4 &controlPoint)
{
    const ImVec2 &rectFrom = ImGui::GetCursorScreenPos(),
                 &avail = ImVec2(256 * m_devicePixelRatio, 256 * m_devicePixelRatio);
    ImVec2 rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
    const ImVec2 curveFrom(rectFrom.x, rectTo.y), curveTo(rectTo.x, rectFrom.y);
    const nanoem_f32_t x0 = rectFrom.x + (controlPoint.x / kInterpolationMaxFloatValue) * avail.x;
    const nanoem_f32_t y0 = rectTo.y - (controlPoint.y / kInterpolationMaxFloatValue) * avail.y;
    const nanoem_f32_t x1 = rectFrom.x + (controlPoint.z / kInterpolationMaxFloatValue) * avail.x;
    const nanoem_f32_t y1 = rectTo.y - (controlPoint.w / kInterpolationMaxFloatValue) * avail.y;
    drawList->AddBezierCubic(curveFrom, ImVec2(x0, y0), ImVec2(x1, y1), curveTo,
        ImGuiWindow::kColorInterpolationCurveBezierLine, m_devicePixelRatio);
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawBezierCurve(
    ImDrawList *drawList, const Vector4 &controlPoint, ImVec2 &controlPointX, ImVec2 &controlPointY)
{
    const ImVec2 &rectFrom = ImGui::GetCursorScreenPos(),
                 &avail = ImVec2(256 * m_devicePixelRatio, 256 * m_devicePixelRatio);
    ImVec2 rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
    const ImVec2 curveFrom(rectFrom.x, rectTo.y), curveTo(rectTo.x, rectFrom.y);
    const nanoem_f32_t x0 = rectFrom.x + (controlPoint.x / kInterpolationMaxFloatValue) * avail.x;
    const nanoem_f32_t y0 = rectTo.y - (controlPoint.y / kInterpolationMaxFloatValue) * avail.y;
    const nanoem_f32_t x1 = rectFrom.x + (controlPoint.z / kInterpolationMaxFloatValue) * avail.x;
    const nanoem_f32_t y1 = rectTo.y - (controlPoint.w / kInterpolationMaxFloatValue) * avail.y;
    controlPointX = ImVec2(x0, y0);
    controlPointY = ImVec2(x1, y1);
    drawList->AddLine(curveFrom, controlPointX, ImGuiWindow::kColorInterpolationCurveControlLine);
    drawList->AddLine(curveTo, controlPointY, ImGuiWindow::kColorInterpolationCurveControlLine);
    drawList->AddBezierCubic(curveFrom, controlPointX, controlPointY, curveTo,
        ImGuiWindow::kColorInterpolationCurveBezierLine, m_devicePixelRatio);
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawXCircle(ImDrawList *drawList, const ImVec2 rectFrom, const ImVec2 avail,
    const ImVec2 &controlPointX, nanoem_f32_t circleRadius, Vector4 &controlPoint)
{
    const ImVec2 circleRect(circleRadius * 2, circleRadius * 2);
    drawList->AddCircleFilled(controlPointX, circleRadius, ImGuiWindow::kColorInterpolationCurveControlPoint);
    ImGui::SetCursorScreenPos(ImVec2(controlPointX.x - circleRadius, controlPointX.y - circleRadius));
    ImGui::InvisibleButton("x", circleRect);
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 pos = ImGui::GetMousePos();
        const Vector2U8 newControlPoint(
            glm::ceil((glm::clamp(pos.x - rectFrom.x, 0.0f, avail.y) / avail.x) * kInterpolationMaxFloatValue),
            kInterpolationMaxFloatValue -
                glm::ceil((glm::clamp(pos.y - rectFrom.y, 0.0f, avail.y) / avail.y) * kInterpolationMaxFloatValue));
        controlPoint.x = newControlPoint.x;
        controlPoint.y = newControlPoint.y;
    }
}

void
BaseKeyframeInterpolationCurveGraphDialog::drawYCircle(ImDrawList *drawList, const ImVec2 rectFrom, const ImVec2 avail,
    const ImVec2 &controlPointY, nanoem_f32_t circleRadius, Vector4 &controlPoint)
{
    const ImVec2 circleRect(circleRadius * 2, circleRadius * 2);
    drawList->AddCircleFilled(controlPointY, circleRadius, ImGuiWindow::kColorInterpolationCurveControlPoint);
    ImGui::SetCursorScreenPos(ImVec2(controlPointY.x - circleRadius, controlPointY.y - circleRadius));
    ImGui::InvisibleButton("y", circleRect);
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        ImVec2 pos = ImGui::GetMousePos();
        const Vector2U8 newControlPoint(
            glm::ceil((glm::clamp(pos.x - rectFrom.x, 0.0f, avail.y) / avail.x) * kInterpolationMaxFloatValue),
            kInterpolationMaxFloatValue -
                glm::ceil((glm::clamp(pos.y - rectFrom.y, 0.0f, avail.y) / avail.y) * kInterpolationMaxFloatValue));
        controlPoint.z = newControlPoint.x;
        controlPoint.w = newControlPoint.y;
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
