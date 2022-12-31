/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BoneParameterDialog.h"

#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const BoneParameterDialog::kIdentifier = "dialog.bone.parameters";

BoneParameterDialog::BoneParameterDialog(Model *model, BaseApplicationService *applicationPtr)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_activeModel(model)
{
    m_activeModel->saveBindPose(m_bindPose);
}

bool
BoneParameterDialog::draw(Project *project)
{
    Model *currentActiveModel = project->activeModel();
    const nanoem_model_bone_t *activeBonePtr = currentActiveModel ? currentActiveModel->activeBone() : nullptr;
    bool visible = activeBonePtr != nullptr;
    if (currentActiveModel != m_activeModel) {
        reset();
        visible = false;
        m_activeModel = currentActiveModel;
        if (m_activeModel) {
            activeBonePtr = m_activeModel->activeBone();
            if (activeBonePtr) {
                m_activeModel->saveBindPose(m_bindPose);
                visible = true;
            }
        }
    }
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 9;
    if (open(tr("nanoem.gui.window.bone.parameter.title"), kIdentifier, &visible, height)) {
        model::Bone *activeBone = model::Bone::cast(activeBonePtr);
        Vector3 translation(activeBone->localUserTranslation()),
            orientation(glm::degrees(glm::eulerAngles(activeBone->localUserOrientation())));
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.bone.parameter.translation"));
        bool translationChanged = false;
        translationChanged |=
            ImGui::DragFloat("##translation.x", &translation.x, ImGuiWindow::kTranslationStepFactor, 0, 0, "X: %.3f");
        translationChanged |=
            ImGui::DragFloat("##translation.y", &translation.y, ImGuiWindow::kTranslationStepFactor, 0, 0, "Y: %.3f");
        translationChanged |=
            ImGui::DragFloat("##translation.z", &translation.z, ImGuiWindow::kTranslationStepFactor, 0, 0, "Z: %.3f");
        if (translationChanged) {
            activeBone->setLocalUserTranslation(translation);
            m_activeModel->performAllBonesTransform();
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.bone.parameter.orientation"));
        bool orientationChanged = false;
        orientationChanged |= ImGui::DragFloat(
            "##orientation.x", &orientation.x, ImGuiWindow::kOrientationStepFactor, -180, 180, "X: %.1f");
        orientationChanged |= ImGui::DragFloat(
            "##orientation.y", &orientation.y, ImGuiWindow::kOrientationStepFactor, -90, 90, "Y: %.1f");
        orientationChanged |= ImGui::DragFloat(
            "##orientation.z", &orientation.z, ImGuiWindow::kOrientationStepFactor, -180, 180, "Z: %.1f");
        if (orientationChanged) {
            activeBone->setLocalUserOrientation(glm::radians(orientation));
            m_activeModel->performAllBonesTransform();
        }
        ImGui::PopItemWidth();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            m_activeModel->registerUpdateActiveBoneTransformCommand(translation, glm::radians(orientation));
            break;
        }
        case kResponseTypeCancel: {
            reset();
            break;
        }
        default:
            break;
        }
    }
    close();
    return visible;
}

void
BoneParameterDialog::reset()
{
    m_activeModel->restoreBindPose(m_bindPose);
    m_activeModel->performAllBonesTransform();
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
