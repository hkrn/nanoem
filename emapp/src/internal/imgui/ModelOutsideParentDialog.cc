/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/ModelOutsideParentDialog.h"

#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const ModelOutsideParentDialog::kIdentifier = "dialog.outside-parent.model";

ModelOutsideParentDialog::ModelOutsideParentDialog(
    Project *project, BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
    , m_activeModel(project->activeModel())
{
}

bool
ModelOutsideParentDialog::draw(Project *project)
{
    bool visible = true;
    Model *currentActiveModel = project->activeModel();
    if (currentActiveModel != m_activeModel) {
        reset();
        m_activeModel = currentActiveModel;
        if (!m_activeModel) {
            visible = false;
        }
    }
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 9;
    if (open(tr("nanoem.gui.window.op.model"), kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.model.subject.bone"));
        const nanoem_model_bone_t *subjectBonePtr = m_activeModel->activeOutsideParentSubjectBone();
        const StringPair targetName(m_activeModel->findOutsideParent(subjectBonePtr));
        const char *subjectBoneName = subjectBonePtr ? model::Bone::cast(subjectBonePtr)->nameConstString()
                                                     : tr("nanoem.gui.window.op.model.subject.bone.none");
        if (ImGui::BeginCombo("##subject", subjectBoneName)) {
            if (ImGui::Selectable(tr("nanoem.gui.window.op.model.subject.bone.none"))) {
                m_activeModel->setActiveOutsideParentSubjectBone(nullptr);
            }
            nanoem_rsize_t numBones;
            nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(m_activeModel->data(), &numBones);
            for (nanoem_rsize_t i = 0; i < numBones; i++) {
                const nanoem_model_bone_t *bonePtr = bones[i];
                if (const model::Bone *bone = model::Bone::cast(bonePtr)) {
                    if (ImGui::Selectable(bone->nameConstString())) {
                        m_activeModel->setActiveOutsideParentSubjectBone(bonePtr);
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.model.target.model"));
        const char *targetModelName =
            targetName.first.empty() ? tr("nanoem.gui.window.op.model.target.model.none") : targetName.first.c_str();
        if (ImGui::BeginCombo("##target.model", targetModelName)) {
            if (ImGui::Selectable(tr("nanoem.gui.window.op.model.target.model.none"))) {
                m_activeModel->setOutsideParent(subjectBonePtr, StringPair());
            }
            const Project::ModelList *models = project->allModels();
            for (Project::ModelList::const_iterator it = models->begin(), end = models->end(); it != end; ++it) {
                Model *model = *it;
                if (ImGui::Selectable(model->nameConstString())) {
                    StringPair newTargetName(targetName);
                    newTargetName.first = model->name();
                    m_activeModel->setOutsideParent(subjectBonePtr, newTargetName);
                }
            }
            ImGui::EndCombo();
        }
        const char *targetBoneName =
            targetName.second.empty() ? tr("nanoem.gui.window.op.model.target.bone.none") : targetName.second.c_str();
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.model.target.bone"));
        if (ImGui::BeginCombo("##target.bone", targetBoneName)) {
            if (ImGui::Selectable(tr("nanoem.gui.window.op.model.target.bone.none"))) {
                StringPair newTargetName(targetName);
                newTargetName.second = String();
                m_activeModel->setOutsideParent(subjectBonePtr, newTargetName);
            }
            if (const Model *targetModel = project->findModelByName(targetModelName)) {
                nanoem_rsize_t numBones;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(targetModel->data(), &numBones);
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    const model::Bone *bone = model::Bone::cast(bonePtr);
                    if (ImGui::Selectable(bone->nameConstString())) {
                        StringPair newTargetName(targetName);
                        newTargetName.second = bone->canonicalName();
                        m_activeModel->setOutsideParent(subjectBonePtr, newTargetName);
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();
        Vector3 translation(0), orientation(0);
        if (const nanoem_model_bone_t *subjectBonePtr = m_activeModel->activeOutsideParentSubjectBone()) {
            const model::Bone *subjectBone = model::Bone::cast(subjectBonePtr);
            translation = subjectBone->localTranslation();
            orientation = glm::eulerAngles(subjectBone->localOrientation());
        }
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.model.subject.bone.translation"));
        ImGui::InputFloat3("##translation", glm::value_ptr(translation), "%.2f", ImGuiInputTextFlags_ReadOnly);
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.model.subject.bone.orientation"));
        ImGui::InputFloat3("##orientation", glm::value_ptr(orientation), "%.2f", ImGuiInputTextFlags_ReadOnly);
        ImGui::Spacing();
        if (m_parent->handleTranslatedButton("nanoem.gui.window.op.model.register.model", -1, true)) {
            CommandRegistrator registrator(project);
            registrator.registerAddModelKeyframesCommandByCurrentLocalFrameIndex(m_activeModel);
        }
        ImGui::Spacing();
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

void
ModelOutsideParentDialog::reset()
{
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
