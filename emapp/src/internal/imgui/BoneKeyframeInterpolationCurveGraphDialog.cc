/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BoneKeyframeInterpolationCurveGraphDialog.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const BoneKeyframeInterpolationCurveGraphDialog::kIdentifier = "dialog.interpolation.bone";

BoneKeyframeInterpolationCurveGraphDialog::BoneKeyframeInterpolationCurveGraphDialog(
    Model *model, Project *project, BaseApplicationService *applicationPtr)
    : BaseKeyframeInterpolationCurveGraphDialog(project, applicationPtr)
    , m_activeModel(model)
    , m_type(project->boneKeyframeInterpolationType())
    , m_controlPoint(model::Bone::kDefaultBezierControlPoint)
{
    if (const model::Bone *bone = model::Bone::cast(m_activeModel->activeBone())) {
        m_controlPoint = bone->bezierControlPoints(m_type);
    }
}

bool
BoneKeyframeInterpolationCurveGraphDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        m_activeModel = currentActiveModel;
        visible = currentActiveModel != nullptr;
    }
    if (visible) {
        nanoem_motion_bone_keyframe_interpolation_type_t type = project->boneKeyframeInterpolationType();
        if (m_type != type) {
            m_type = type;
        }
    }
    if (open(tr("nanoem.gui.window.interpolation.title"), kIdentifier, &visible,
            256 * m_devicePixelRatio + ImGui::GetFrameHeightWithSpacing())) {
        model::Bone *activeBone = nullptr;
#if 0
        Vector4 controlPoint(0);
        Motion::BoneKeyframeList keyframes;
        Motion *motion = project->resolveMotion(m_activeModel);
        motion->selection()->getAll(keyframes, nullptr);
        if (keyframes.size() > 1) {
            const ImVec2 rectFrom(ImGui::GetCursorScreenPos()),
                avail(256 * m_devicePixelRatio, 256 * m_devicePixelRatio),
                rectTo(rectFrom.x + avail.x, rectFrom.y + avail.y);
            ImDrawList *drawList = ImGui::GetWindowDrawList();
            drawList->PushClipRect(rectFrom, rectTo);
            drawBaseGrid(drawList);
            for (Motion::BoneKeyframeList::const_iterator it = keyframes.begin(), end = keyframes.end(); it != end;
                 ++it) {
                const nanoem_motion_bone_keyframe_t *keyframe = *it;
                const Vector4 controlPoint(glm::make_vec4(nanoemMotionBoneKeyframeGetInterpolation(keyframe, m_type)));
                drawBezierCurve(drawList, controlPoint);
            }
            drawList->AddRect(rectFrom, rectTo, ImGuiWindow::kColorBorder);
            drawList->PopClipRect();
            ImGui::Dummy(avail);
        }
        else if (keyframes.size() == 1) {
            const nanoem_motion_bone_keyframe_t *keyframe = keyframes.front();
            const nanoem_model_bone_t *activeBonePtr =
                m_activeModel->findBone(nanoemMotionBoneKeyframeGetName(keyframes.front()));
            activeBone = model::Bone::cast(activeBonePtr);
            controlPoint = activeBone->bezierControlPoints(m_type);
            nanoem_motion_bone_keyframe_t *next = nullptr;
            const nanoem_unicode_string_t *namePtr =
                nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
            const Motion *motion = project->resolveMotion(m_activeModel);
            nanoemMotionSearchClosestBoneKeyframes(motion->data(), namePtr, frameIndex, nullptr, &next);
            const nanoem_frame_index_t current = project->currentLocalFrameIndex(),
                                       start = nanoemMotionKeyframeObjectGetFrameIndex(
                                           nanoemMotionBoneKeyframeGetKeyframeObject(keyframe)),
                                       end = nanoemMotionKeyframeObjectGetFrameIndex(
                                           nanoemMotionBoneKeyframeGetKeyframeObject(next)),
                                       interval = end - start;
            nanoem_f32_t coef = Motion::coefficient(keyframe, next, current);
            drawSingleKeyframeGraph(interval, coef, controlPoint);
        }
        else {
            drawEmptyGraph();
        }
#else
        const nanoem_model_bone_t *activeBonePtr = m_activeModel->activeBone();
        activeBone = model::Bone::cast(m_activeModel->activeBone());
        Vector4 controlPoint(activeBone->bezierControlPoints(m_type));
        nanoem_motion_bone_keyframe_t *prev = nullptr, *next = nullptr;
        const nanoem_unicode_string_t *namePtr = nanoemModelBoneGetName(activeBonePtr, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
        const nanoem_frame_index_t frameIndex = project->currentLocalFrameIndex();
        const Motion *motion = project->resolveMotion(m_activeModel);
        nanoemMotionSearchClosestBoneKeyframes(motion->data(), namePtr, frameIndex, &prev, &next);
        nanoem_frame_index_t interval =
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(next)) -
            nanoemMotionKeyframeObjectGetFrameIndex(nanoemMotionBoneKeyframeGetKeyframeObject(prev));
        nanoem_f32_t coef = Motion::coefficient(prev, next, frameIndex);
        drawSingleKeyframeGraph(interval * 1.0f, coef, controlPoint);
#endif
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeCancel: {
            if (activeBone) {
                activeBone->setBezierControlPoints(m_type, m_controlPoint);
            }
            break;
        }
        case kResponseTypeOK:
        default:
            if (activeBone) {
                activeBone->setBezierControlPoints(m_type, controlPoint);
            }
            break;
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
