/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/CameraKeyframeInterpolationCurveGraphDialog.h"

#include "emapp/ICamera.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const CameraKeyframeInterpolationCurveGraphDialog::kIdentifier = "dialog.interpolation.camera";

CameraKeyframeInterpolationCurveGraphDialog::CameraKeyframeInterpolationCurveGraphDialog(
    Project *project, BaseApplicationService *applicationPtr)
    : BaseKeyframeInterpolationCurveGraphDialog(project, applicationPtr)
    , m_type(project->cameraKeyframeInterpolationType())
    , m_controlPoint(project->globalCamera()->bezierControlPoints(m_type))
{
}

bool
CameraKeyframeInterpolationCurveGraphDialog::draw(Project *project)
{
    bool visible = true;
    nanoem_motion_camera_keyframe_interpolation_type_t type = project->cameraKeyframeInterpolationType();
    if (m_type != type) {
        m_type = type;
        m_controlPoint = project->globalCamera()->bezierControlPoints(m_type);
    }
    if (open(tr("nanoem.gui.window.interpolation.title"), kIdentifier, &visible,
            256 * m_devicePixelRatio + ImGui::GetFrameHeightWithSpacing())) {
        const Motion *motion = project->cameraMotion();
        Vector4 controlPoint(project->globalCamera()->bezierControlPoints(m_type));
#if 0
        Motion::CameraKeyframeList keyframes;
        motion->selection()->getAll(keyframes, nullptr);
        if (keyframes.size() > 1) {
            const ImVec2 rectFrom(ImGui::GetCursorScreenPos()),
                avail(256 * m_devicePixelRatio, 256 * m_devicePixelRatio),
                rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(rectFrom, rectTo);
            drawBaseGrid(drawList);
            for (Motion::CameraKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_camera_keyframe_t *keyframe = *it;
                const Vector4 controlPoint(glm::make_vec4(nanoemMotionCameraKeyframeGetInterpolation(keyframe, m_type)));
                drawBezierCurve(drawList, controlPoint);
            }
            drawList->AddRect(rectFrom, rectTo, ImGuiWindow::kColorBorder);
            drawList->PopClipRect();
            ImGui::Dummy(avail);
        }
        else if (keyframes.size() == 1) {
            const nanoem_motion_camera_keyframe_t *keyframe = keyframes.front();
            const nanoem_frame_index_t frameIndex =
                nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(keyframe));
            nanoem_motion_camera_keyframe_t *next = nullptr;
            nanoemMotionSearchClosestCameraKeyframes(motion->data(), frameIndex, nullptr, &next);
            const nanoem_frame_index_t current = project->currentLocalFrameIndex(),
                                       start = nanoemMotionKeyframeObjectGetFrameIndex(
                                           nanoemMotionCameraKeyframeGetKeyframeObject(keyframe)),
                                       end = nanoemMotionKeyframeObjectGetFrameIndex(
                                           nanoemMotionCameraKeyframeGetKeyframeObject(next)),
                                       interval = end - start;
            nanoem_f32_t coef = Motion::coefficient(keyframe, next, current);
            drawSingleKeyframeGraph(interval, coef, controlPoint);
        }
        else {
            drawEmptyGraph();
        }
#else
        const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
        nanoem_motion_camera_keyframe_t *prev = nullptr, *next = nullptr;
        nanoemMotionSearchClosestCameraKeyframes(motion->data(), frameIndex, &prev, &next);
        nanoem_frame_index_t interval =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(next)) -
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionCameraKeyframeGetKeyframeObject(prev));
        nanoem_f32_t coef = Motion::coefficient(prev, next, frameIndex);
        drawSingleKeyframeGraph(interval * 1.0f, coef, controlPoint);
#endif
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeCancel: {
            project->globalCamera()->setBezierControlPoints(m_type, m_controlPoint);
            break;
        }
        case kResponseTypeOK:
        default:
            project->globalCamera()->setBezierControlPoints(m_type, controlPoint);
            break;
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
