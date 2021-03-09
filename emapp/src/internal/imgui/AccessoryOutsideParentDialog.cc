/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/AccessoryOutsideParentDialog.h"

#include "emapp/Accessory.h"
#include "emapp/CommandRegistrator.h"
#include "emapp/Model.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const AccessoryOutsideParentDialog::kIdentifier = "dialog.outside-parent.accessory";

AccessoryOutsideParentDialog::AccessoryOutsideParentDialog(
    Accessory *accessory, BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
    , m_activeAccessory(accessory)
{
}

bool
AccessoryOutsideParentDialog::draw(Project *project)
{
    bool visible = true;
    Accessory *currentActiveAccessory = project->activeAccessory();
    if (currentActiveAccessory != m_activeAccessory) {
        reset();
        m_activeAccessory = currentActiveAccessory;
        if (!m_activeAccessory) {
            visible = false;
        }
    }
    const nanoem_f32_t height = ImGui::GetFrameHeightWithSpacing() * 6;
    if (open(tr("nanoem.gui.window.op.accessory"), kIdentifier, &visible, height)) {
        ImGui::PushItemWidth(-1);
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.accessory.target.model"));
        StringPair currentOutsideParent(m_activeAccessory->outsideParent());
        const char *targetModelName = currentOutsideParent.first.empty()
            ? tr("nanoem.gui.window.op.accessory.target.model.none")
            : currentOutsideParent.first.c_str();
        if (ImGui::BeginCombo("##target.model", targetModelName)) {
            if (ImGui::Selectable(tr("nanoem.gui.window.op.accessory.target.model.none"))) {
                m_activeAccessory->setOutsideParent(StringPair());
            }
            const Project::ModelList models(project->allModels());
            for (Project::ModelList::const_iterator it = models.begin(), end = models.end(); it != end; ++it) {
                Model *model = *it;
                if (ImGui::Selectable(model->nameConstString())) {
                    StringPair newOutsideParent(currentOutsideParent);
                    newOutsideParent.first = model->name();
                    m_activeAccessory->setOutsideParent(newOutsideParent);
                }
            }
            ImGui::EndCombo();
        }
        const char *targetBoneName = currentOutsideParent.second.empty()
            ? tr("nanoem.gui.window.op.accessory.target.bone.none")
            : currentOutsideParent.second.c_str();
        ImGui::TextUnformatted(tr("nanoem.gui.window.op.accessory.target.bone"));
        if (ImGui::BeginCombo("##target.bone", targetBoneName)) {
            if (ImGui::Selectable(tr("nanoem.gui.window.op.accessory.target.bone.none"))) {
                StringPair newOutsideParent(currentOutsideParent);
                newOutsideParent.second = String();
                m_activeAccessory->setOutsideParent(newOutsideParent);
            }
            if (const Model *targetModel = project->findModelByName(targetModelName)) {
                nanoem_rsize_t numBones;
                nanoem_model_bone_t *const *bones = nanoemModelGetAllBoneObjects(targetModel->data(), &numBones);
                for (nanoem_rsize_t i = 0; i < numBones; i++) {
                    const nanoem_model_bone_t *bonePtr = bones[i];
                    const model::Bone *bone = model::Bone::cast(bonePtr);
                    if (ImGui::Selectable(bone->nameConstString())) {
                        StringPair newOutsideParent(currentOutsideParent);
                        newOutsideParent.second = bone->canonicalName();
                        m_activeAccessory->setOutsideParent(newOutsideParent);
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Spacing();
        if (m_parent->handleTranslatedButton("nanoem.gui.window.op.accessory.register", -1, true)) {
            CommandRegistrator registrator(project);
            registrator.registerAddAccessoryKeyframesCommandByCurrentLocalFrameIndex(m_activeAccessory);
        }
        ImGui::Spacing();
        ImGui::PopItemWidth();
    }
    close();
    return visible;
}

void
AccessoryOutsideParentDialog::reset()
{
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
