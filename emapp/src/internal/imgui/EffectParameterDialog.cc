/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/EffectParameterDialog.h"

#include "emapp/Accessory.h"
#include "emapp/Effect.h"
#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/Model.h"
#include "emapp/Progress.h"
#include "emapp/internal/ModelEffectSetting.h"
#include "emapp/private/CommonInclude.h"

extern "C" {
#include "wildcardcmp.h"
}

namespace nanoem {
namespace internal {
namespace imgui {

const char *const EffectParameterDialog::kIdentifier = "dialog.project.effect";
const nanoem_f32_t EffectParameterDialog::kMinimumWindowWidth = 600;

void
EffectParameterDialog::handleLoadingModelEffectSetting(
    const URI &fileURI, Project *project, Error &error, void *userData)
{
    FileReaderScope scope(project->translator());
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        Progress progress(project, 0);
        FileUtils::read(scope, bytes, error);
        Model *activeModel = static_cast<Model *>(userData);
        bytes.push_back(0);
        internal::ModelEffectSetting settings(project);
        settings.load(reinterpret_cast<const char *>(bytes.data()), activeModel, progress, error);
    }
}

void
EffectParameterDialog::handleSaveingModelEffectSetting(
    const URI &fileURI, Project *project, Error &error, void *userData)
{
    const Model *activeModel = static_cast<const Model *>(userData);
    internal::ModelEffectSetting settings(project);
    MutableString content;
    settings.save(activeModel, content);
    FileWriterScope scope;
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::write(scope.writer(), content.data(), content.size(), error);
        error.hasReason() ? scope.rollback(error) : scope.commit(error);
    }
}

EffectParameterDialog::EffectParameterDialog(BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
    , m_activeOffscreenRenderTargetIndex(0)
    , m_activeModelTargetIndex(-1)
    , m_activeParameterIndex(0)
{
}

bool
EffectParameterDialog::draw(Project *project)
{
    bool visible = true;
    nanoem_f32_t width = kMinimumWindowWidth * project->windowDevicePixelRatio();
    Effect *effect = nullptr;
    IDrawable *drawablePtr = nullptr;
    if (Effect *modelEffect = project->resolveEffect(project->activeModel())) {
        effect = modelEffect;
        drawablePtr = project->activeModel();
    }
    else if (Effect *accessoryEffect = project->resolveEffect(project->activeAccessory())) {
        effect = accessoryEffect;
        drawablePtr = project->activeAccessory();
    }
    char title[Inline::kLongNameStackBufferSize];
    if (drawablePtr) {
        StringUtils::format(title, sizeof(title), "%s - %s", tr("nanoem.gui.window.project.effect.title"),
            drawablePtr->nameConstString());
    }
    else {
        StringUtils::copyString(title, tr("nanoem.gui.window.project.effect.title"), sizeof(title));
    }
    if (open(title, kIdentifier, &visible, ImVec2(width, ImGui::GetFrameHeightWithSpacing() * 16),
            ImGuiWindowFlags_None)) {
        ImGui::BeginTabBar("tabbar");
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.project.effect.tab.offscreen"))) {
            layoutAllOffscreenRenderTargets(project);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.project.effect.tab.model"))) {
            layoutAllModelMaterialEffectAttachments(project);
            ImGui::EndTabItem();
        }
        if (effect && ImGui::BeginTabItem(tr("nanoem.gui.window.project.effect.tab.parameter"))) {
            bool needsReload = false;
            layoutAllParameters(project, effect, needsReload);
            if (needsReload) {
                m_parent->reloadActiveEffect(drawablePtr);
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    else {
        project->clearAllDrawablesToAttachOffscreenRenderTargetEffect(String());
    }
    close();
    return visible;
}

void
EffectParameterDialog::layoutAllOffscreenRenderTargets(Project *project)
{
    const Project::DrawableList *drawables = project->drawableOrderList();
    typedef tinystl::pair<effect::OffscreenRenderTargetOption, const Effect *> OffscreenRenderTargetPair;
    typedef tinystl::vector<OffscreenRenderTargetPair, TinySTLAllocator> OffscreenRenderTargetPairList;
    OffscreenRenderTargetPairList allOptions;
    nanoem_f32_t maxTextWidth = 0;
    for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
        const IDrawable *drawable = *it;
        if (const Effect *effect = project->resolveEffect(drawable)) {
            effect::OffscreenRenderTargetOptionList options;
            effect->getAllOffscreenRenderTargetOptions(options);
            for (effect::OffscreenRenderTargetOptionList::const_iterator it2 = options.begin(), end2 = options.end();
                 it2 != end2; ++it2) {
                const effect::OffscreenRenderTargetOption &option = *it2;
                maxTextWidth = glm::max(maxTextWidth, ImGui::CalcTextSize(option.m_name.c_str()).x);
                if (drawable->isVisible()) {
                    allOptions.push_back(tinystl::make_pair(option, effect));
                }
            }
        }
        else {
            maxTextWidth = glm::max(maxTextWidth, ImGui::CalcTextSize(drawable->nameConstString()).x);
        }
    }
    ImGui::BeginChild("left-pane", ImVec2(ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio(), 0), true);
    ImGuiListClipper clipper;
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, allOptions.size() + 1, m_activeOffscreenRenderTargetIndex);
    clipper.Begin(Inline::saturateInt32(allOptions.size() + 1));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const char *name =
                i == 0 ? Effect::kOffscreenOwnerNameMain.c_str() : allOptions[i - 1].first.m_name.c_str();
            const bool selected = m_activeOffscreenRenderTargetIndex == i;
            if (ImGui::Selectable(name, selected) || ((up || down) && selected)) {
                m_activeOffscreenRenderTargetIndex = i;
            }
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    ImGui::PushItemWidth(-1);
    if (m_activeOffscreenRenderTargetIndex > 0 &&
        m_activeOffscreenRenderTargetIndex <= Inline::saturateInt32(allOptions.size())) {
        const OffscreenRenderTargetPair &item = allOptions[m_activeOffscreenRenderTargetIndex - 1];
        layoutAllOffscreenRenderTargetAttachments(project, item.second, item.first, maxTextWidth);
    }
    else {
        layoutOffscreenMainRenderTargetAttachments(project, maxTextWidth);
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
EffectParameterDialog::layoutOffscreenMainRenderTargetAttachments(Project *project, nanoem_f32_t maxTextWidth)
{
    const char descPtr[] = "Main Render Target";
    MutableString desc(descPtr, descPtr + sizeof(descPtr));
    desc.push_back(0);
    ImGui::InputTextMultiline("##desc", desc.data(), desc.size(), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 3),
        ImGuiInputTextFlags_ReadOnly);
    layoutDefaultOffscreenRenderTargetAttachment(project, Effect::kOffscreenOwnerNameMain);
    const Project::DrawableList *drawables = project->drawableOrderList();
    for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
        IDrawable *drawable = *it;
        char buffer[Inline::kLongNameStackBufferSize];
        StringUtils::format(
            buffer, sizeof(buffer), "##%s/%p/visible", Effect::kOffscreenOwnerNameMain.c_str(), drawable);
        bool visible = drawable->isVisible();
        if (ImGuiWindow::handleCheckBox(buffer, &visible, true)) {
            drawable->setVisible(visible);
        }
        layoutOffscreenRenderTargetAttachment(project, drawable, Effect::kOffscreenOwnerNameMain, maxTextWidth);
    }
}

void
EffectParameterDialog::layoutAllOffscreenRenderTargetAttachments(Project *project, const Effect *ownerEffect,
    const effect::OffscreenRenderTargetOption &option, nanoem_f32_t maxTextWidth)
{
    const String &d = option.m_description;
    MutableString desc(d.c_str(), d.c_str() + d.size());
    desc.push_back(0);
    ImGui::InputTextMultiline("##desc", desc.data(), desc.size(), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 3),
        ImGuiInputTextFlags_ReadOnly);
    layoutDefaultOffscreenRenderTargetAttachment(project, option.m_name);
    const Project::DrawableList *drawables = project->drawableOrderList();
    for (Project::DrawableList::const_iterator it = drawables->begin(), end = drawables->end(); it != end; ++it) {
        IDrawable *drawable = *it;
        const IEffect *activeEffect = drawable->activeEffect();
        char buffer[Inline::kLongNameStackBufferSize];
        const String &name = option.m_name;
        StringUtils::format(buffer, sizeof(buffer), "##%s/%p/visible", name.c_str(), drawable);
        const Effect *effect = project->upcastEffect(drawable->findOffscreenPassiveRenderTargetEffect(name.c_str()));
        bool enabled = drawable->isOffscreenPassiveRenderTargetEffectEnabled(name);
        if (ImGuiWindow::handleCheckBox(buffer, &enabled, effect != nullptr)) {
            drawable->setOffscreenPassiveRenderTargetEffectEnabled(name, enabled);
        }
        layoutOffscreenRenderTargetAttachment(project, drawable, name, maxTextWidth);
    }
    addSeparator();
    const ImTextureID textureID = reinterpret_cast<ImTextureID>(option.m_colorImage.id);
    if (ImGui::TreeNode(textureID, "%s", tr("nanoem.gui.window.project.effect.offscreen.display-texture"))) {
        ImVec2 uv0, uv1;
        ImGuiWindow::getImageCoordinate(uv0, uv1);
        ImGui::Image(textureID, calcExpandedImageSize(option.m_colorImageDescription, 1.0f), uv0, uv1,
            ImVec4(1, 1, 1, 1), ImGui::ColorConvertU32ToFloat4(ImGuiWindow::kColorBorder));
        ImGui::TreePop();
    }
}

void
EffectParameterDialog::layoutDefaultOffscreenRenderTargetAttachment(Project *project, const String &offscreenOwnerName)
{
#if 0
    bool dummy = false;
    ImGuiWindow::handleCheckBox("##dummy", &dummy, false);
    ImGui::SameLine();
    ImGui::Text(tr("nanoem.gui.window.project.effect.offscreen.default"));
    ImGui::SameLine();
    if (m_parent->handleTranslatedButton("nanoem.gui.window.project.effect.offscreen.none", -1, true)) {
        project->setOffscreenRenderTargetDrawable(
                    tinystl::make_pair(offscreenOwnerName, static_cast<IDrawable *>(nullptr)));
        queryFileDialog(IFileManager::kDialogTypeOpenEffectFile, project);
    }
#else
    BX_UNUSED_2(project, offscreenOwnerName);
#endif
}

void
EffectParameterDialog::layoutOffscreenRenderTargetAttachment(
    Project *project, IDrawable *drawable, const String &offscreenOwnerName, nanoem_f32_t maxTextWidth)
{
    static const nanoem_u8_t kClearIcon[] = { 0xEF, 0x84, 0xAD, 0x0 };
    char buffer[Inline::kLongNameStackBufferSize];
    const Effect *effect =
        project->upcastEffect(drawable->findOffscreenPassiveRenderTargetEffect(offscreenOwnerName.c_str()));
    nanoem_f32_t devicePixelRatio = project->windowDevicePixelRatio();
    ImGui::SameLine();
    const char *drawableName = drawable->nameConstString();
    const bool selectable = drawable->activeEffect() != effect ||
        (effect && effect->scriptOrder() == IEffect::kScriptOrderTypeStandard),
               wasSelected = project->containsDrawableToAttachOffscreenRenderTargetEffect(offscreenOwnerName, drawable);
    if (wasSelected) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
    }
    bool selected = wasSelected;
    if (ImGui::Selectable(drawableName, &selected,
            selectable ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_Disabled, ImVec2(maxTextWidth, 0))) {
        if (selected) {
            if (!ImGui::GetIO().KeyShift) {
                project->clearAllDrawablesToAttachOffscreenRenderTargetEffect(offscreenOwnerName);
            }
            project->addDrawableToAttachOffscreenRenderTargetEffect(offscreenOwnerName, drawable);
        }
        else {
            project->removeDrawableToAttachOffscreenRenderTargetEffect(offscreenOwnerName, drawable);
        }
    }
    if (wasSelected) {
        ImGui::PopStyleColor();
    }
    ImGui::SameLine();
    const ImGuiStyle &style = ImGui::GetStyle();
    const nanoem_f32_t clearIconWidth = ImGui::CalcTextSize(reinterpret_cast<const char *>(kClearIcon)).x * 1.5f,
                       openButtonWidth = ImGui::GetContentRegionAvail().x -
        (style.ItemSpacing.x + style.FramePadding.x * 2 + clearIconWidth * devicePixelRatio);
    if (effect) {
        const URI &fileURI = effect->fileURI();
        const String &filename =
            fileURI.hasFragment() ? URI::lastPathComponent(fileURI.fragment()) : fileURI.lastPathComponent();
        StringUtils::format(buffer, sizeof(buffer), "%s##%s/%p/%s/attach", filename.c_str(), offscreenOwnerName.c_str(),
            drawable, fileURI.absolutePathConstString());
        if (ImGuiWindow::handleButton(buffer, openButtonWidth, selectable)) {
            project->addDrawableToAttachOffscreenRenderTargetEffect(offscreenOwnerName, drawable);
            queryFileDialog(IFileManager::kDialogTypeOpenEffectFile, project);
        }
        else if (ImGui::GetIO().KeyShift && ImGui::IsItemHovered()) {
            m_parent->drawTextTooltip(fileURI.absolutePathConstString());
        }
        ImGui::SameLine();
        StringUtils::format(buffer, sizeof(buffer), "%s##%s/%p/%s/clear", kClearIcon, offscreenOwnerName.c_str(),
            drawable, fileURI.absolutePathConstString());
        if (ImGuiWindow::handleButton(buffer, clearIconWidth, selectable)) {
            drawable->removeOffscreenPassiveRenderTargetEffect(offscreenOwnerName);
        }
    }
    else {
        const char *noneText = tr("nanoem.gui.window.project.effect.offscreen.none");
        StringUtils::format(buffer, sizeof(buffer), "%s##%s/%p/%s/attach", noneText, offscreenOwnerName.c_str(),
            drawable, offscreenOwnerName.c_str());
        if (ImGuiWindow::handleButton(buffer, openButtonWidth, true)) {
            project->clearAllDrawablesToAttachOffscreenRenderTargetEffect(offscreenOwnerName);
            project->addDrawableToAttachOffscreenRenderTargetEffect(offscreenOwnerName, drawable);
            queryFileDialog(IFileManager::kDialogTypeOpenEffectFile, project);
        }
        ImGui::SameLine();
        StringUtils::format(buffer, sizeof(buffer), "%s##%s/%p/%s/clear", kClearIcon, offscreenOwnerName.c_str(),
            drawable, offscreenOwnerName.c_str());
        ImGuiWindow::handleButton(buffer, clearIconWidth, false);
    }
}

void
EffectParameterDialog::layoutAllModelMaterialEffectAttachments(Project *project)
{
    const Project::ModelList *models = project->allModels();
    const nanoem_rsize_t numModels = models->size();
    const float width = ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio();
    ImGui::BeginChild("left-pane", ImVec2(width, 0), false);
    float height = ImGui::GetContentRegionAvail().y;
    height -= ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("left-pane-inner", ImVec2(width, height), true);
    ImGuiListClipper clipper;
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, models->size(), m_activeModelTargetIndex);
    clipper.Begin(Inline::saturateInt32(numModels));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const bool selected = m_activeModelTargetIndex == i;
            if (ImGui::Selectable(models->data()[i]->nameConstString(), selected) || ((up || down) && selected)) {
                ImGui::SetScrollHereY();
                m_activeModelTargetIndex = i;
            }
        }
    }
    ImGui::EndChild();
    bool isModelSelected = m_activeModelTargetIndex >= 0 && m_activeModelTargetIndex < Inline::saturateInt32(numModels);
    if (ImGuiWindow::handleButton(tr("nanoem.gui.window.project.effect.emd.load"),
            ImGui::GetContentRegionAvail().x * 0.5f, isModelSelected)) {
        Model *model = models->data()[m_activeModelTargetIndex];
        const IFileManager::QueryFileDialogCallbacks callbacks = { model, handleLoadingModelEffectSetting, nullptr,
            nullptr };
        project->fileManager()->setTransientQueryFileDialogCallback(callbacks);
        StringList extensions;
        extensions.push_back("emd");
        IEventPublisher *eventPublisher = project->eventPublisher();
        eventPublisher->publishQueryOpenSingleFileDialogEvent(IFileManager::kDialogTypeUserCallback, extensions);
    }
    ImGui::SameLine();
    if (ImGuiWindow::handleButton(
            tr("nanoem.gui.window.project.effect.emd.save"), ImGui::GetContentRegionAvail().x, isModelSelected)) {
        Model *model = models->data()[m_activeModelTargetIndex];
        const IFileManager::QueryFileDialogCallbacks callbacks = { model, handleSaveingModelEffectSetting, nullptr,
            nullptr };
        project->fileManager()->setTransientQueryFileDialogCallback(callbacks);
        StringList extensions;
        extensions.push_back("emd");
        IEventPublisher *eventPublisher = project->eventPublisher();
        eventPublisher->publishQuerySaveFileDialogEvent(IFileManager::kDialogTypeUserCallback, extensions);
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    ImGui::PushItemWidth(-1);
    if (isModelSelected) {
        static const nanoem_u8_t kClearIcon[] = { 0xEF, 0x84, 0xAD, 0x0 };
        const ImGuiStyle &style = ImGui::GetStyle();
        Model *model = models->data()[m_activeModelTargetIndex];
        nanoem_rsize_t numMaterials;
        nanoem_model_material_t *const *materials = nanoemModelGetAllMaterialObjects(model->data(), &numMaterials);
        nanoem_f32_t maxTextWidth = 0;
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const nanoem_model_material_t *materialPtr = materials[i];
            model::Material *material = model::Material::cast(materialPtr);
            maxTextWidth = glm::max(maxTextWidth, ImGui::CalcTextSize(material->nameConstString()).x);
        }
        const nanoem_u16_t modelHandle = model->handle();
        for (nanoem_rsize_t i = 0; i < numMaterials; i++) {
            const char *noneText = tr("nanoem.gui.window.project.effect.material.none");
            const nanoem_model_material_t *materialPtr = materials[i];
            const nanoem_rsize_t materialIndex = static_cast<nanoem_rsize_t>(i);
            model::Material *material = model::Material::cast(materialPtr);
            const char *materialName = material->nameConstString();
            const bool wasSelected = project->containsIndexOfMaterialToAttachEffect(modelHandle, materialIndex);
            if (wasSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImGuiWindow::kColorSelectedModelObject);
            }
            bool selected = wasSelected;
            if (ImGui::Selectable(materialName, &selected, ImGuiSelectableFlags_None, ImVec2(maxTextWidth, 0))) {
                if (selected) {
                    if (!ImGui::GetIO().KeyShift) {
                        project->clearAllIndicesOfMaterialToAttachEffect(modelHandle);
                    }
                    project->addIndexOfMaterialToAttachEffect(modelHandle, materialIndex);
                }
                else {
                    project->removeIndexOfMaterialToAttachEffect(modelHandle, materialIndex);
                }
            }
            if (wasSelected) {
                ImGui::PopStyleColor();
            }
            ImGui::SameLine();
            const Effect *effect = material->effect();
            const char *name = effect ? effect->nameConstString() : noneText;
            char buffer[Inline::kMarkerStringLength];
            StringUtils::format(buffer, sizeof(buffer), "%s##nanoem.gui.window.project.effect.model.%s.%d", name,
                model->canonicalNameConstString(), Inline::saturateInt32U(i));
            const nanoem_f32_t clearIconWidth =
                                   ImGui::CalcTextSize(reinterpret_cast<const char *>(kClearIcon)).x * 1.5f,
                               openButtonWidth = ImGui::GetContentRegionAvail().x -
                (style.ItemSpacing.x + style.FramePadding.x * 2 + clearIconWidth * project->windowDevicePixelRatio());
            if (ImGuiWindow::handleButton(buffer, openButtonWidth, true)) {
                project->clearAllIndicesOfMaterialToAttachEffect(modelHandle);
                project->addIndexOfMaterialToAttachEffect(modelHandle, materialIndex);
                queryFileDialog(IFileManager::kDialogTypeOpenEffectFile, project);
            }
            else if (effect && ImGui::GetIO().KeyShift && ImGui::IsItemHovered()) {
                m_parent->drawTextTooltip(effect->fileURI().absolutePathConstString());
            }
            ImGui::SameLine();
            StringUtils::format(buffer, sizeof(buffer), "%s##nanoem.gui.window.project.effect.model.%s.%d.clear",
                kClearIcon, model->canonicalNameConstString(), Inline::saturateInt32U(i));
            if (ImGuiWindow::handleButton(buffer, clearIconWidth, material->effect() != nullptr)) {
                material->setEffect(nullptr);
            }
        }
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
EffectParameterDialog::layoutAllParameters(Project *project, Effect *effect, bool &needsReload)
{
    static const String kDefault("Default");
    StringPtrList names;
    names.push_back(&kDefault);
    Effect::PassUniformBufferMap uniformBuffers;
    if (effect->isPassUniformBufferInspectionEnabled()) {
        effect->getPassUniformBuffer(uniformBuffers);
        for (Effect::PassUniformBufferMap::const_iterator it = uniformBuffers.begin(), end = uniformBuffers.end();
             it != end; ++it) {
            names.push_back(&it->first);
        }
    }
    int uniformBuffersBoundIndex = Inline::saturateInt32(names.size());
    Effect::NamedRenderTargetColorImageContainerMap renderTargets;
    effect->getAllRenderTargetImageContainers(renderTargets);
    for (Effect::NamedRenderTargetColorImageContainerMap::const_iterator it = renderTargets.begin(),
                                                                         end = renderTargets.end();
         it != end; ++it) {
        names.push_back(&it->first);
    }
    int renderTargetsBoundIndex = Inline::saturateInt32(names.size());
    effect::OffscreenRenderTargetOptionList offscreenRenderTargets;
    effect->getAllOffscreenRenderTargetOptions(offscreenRenderTargets);
    for (effect::OffscreenRenderTargetOptionList::const_iterator it = offscreenRenderTargets.begin(),
                                                                 end = offscreenRenderTargets.end();
         it != end; ++it) {
        names.push_back(&it->m_name);
    }
    if (!effect->fileURI().hasFragment()) {
        if (m_parent->handleTranslatedButton("nanoem.gui.window.project.effect.parameter.reload")) {
            needsReload = true;
        }
        ImGui::SameLine();
    }
    bool enabled = effect->isPassUniformBufferInspectionEnabled();
    if (ImGuiWindow::handleCheckBox(tr("nanoem.gui.window.project.effect.parameter.inspect"), &enabled, true)) {
        effect->setPassUniformBufferInspectionEnabled(enabled);
        m_activeParameterIndex = 0;
    }
    ImGui::BeginChild("left-pane", ImVec2(ImGuiWindow::kLeftPaneWidth * project->windowDevicePixelRatio(), 0), true);
    ImGuiListClipper clipper;
    bool up, down;
    detectUpDown(up, down);
    selectIndex(up, down, names.size(), m_activeParameterIndex);
    clipper.Begin(Inline::saturateInt32(names.size()));
    while (clipper.Step()) {
        for (int i = clipper.DisplayStart, end = clipper.DisplayEnd; i < end; i++) {
            const bool selected = m_activeParameterIndex == i;
            if (ImGui::Selectable(names[i]->c_str(), selected) || ((up || down) && selected)) {
                ImGui::SetScrollHereY();
                m_activeParameterIndex = i;
            }
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("right-pane", ImGui::GetContentRegionAvail());
    ImGui::PushItemWidth(-1);
    if (m_activeParameterIndex > 0 && m_activeParameterIndex < Inline::saturateInt32(names.size())) {
        const String &name = *names[m_activeParameterIndex];
        if (m_activeParameterIndex >= renderTargetsBoundIndex) {
            const effect::OffscreenRenderTargetOption *it =
                &offscreenRenderTargets[m_activeParameterIndex - renderTargetsBoundIndex];
            const sg_image_desc &desc = it->m_colorImageDescription;
            const Vector2UI16 size(desc.width, desc.height);
            layoutRenderTargetImage(it->m_colorImage, size, it->m_name, it->m_description);
        }
        else if (m_activeParameterIndex >= uniformBuffersBoundIndex) {
            Effect::NamedRenderTargetColorImageContainerMap::const_iterator it = renderTargets.find(name);
            if (it != renderTargets.end()) {
                const effect::RenderTargetColorImageContainer *container = it->second;
                const sg_image_desc &desc = container->colorImageDescription();
                const Vector2UI16 size(desc.width, desc.height);
                layoutRenderTargetImage(container->colorImageHandle(), size, it->first, String());
            }
            else {
                ImGui::TextUnformatted("(Unknown)");
            }
        }
        else if (!uniformBuffers.empty()) {
            Effect::PassUniformBufferMap::const_iterator it = uniformBuffers.find(name);
            if (it != uniformBuffers.end()) {
                const Effect::NamedByteArrayMap &map = it->second;
                StringPtrList parameterNames;
                for (Effect::NamedByteArrayMap::const_iterator it2 = map.begin(), end2 = map.end(); it2 != end2;
                     ++it2) {
                    parameterNames.push_back(&it2->first);
                }
                StringPtrListSort sort;
                sort.execute(parameterNames);
                for (StringPtrList::const_iterator it2 = parameterNames.begin(), end2 = parameterNames.end();
                     it2 != end2; ++it2) {
                    const String *name2 = *it2;
                    const ByteArray &bytes = map.find(*name2)->second;
                    layoutUniformBufferDetail(*name2, bytes);
                }
            }
        }
    }
    else {
        effect::UIWidgetParameterList parameters;
        effect->getAllUIWidgetParameters(parameters);
        for (effect::UIWidgetParameterList::const_iterator it = parameters.begin(), end = parameters.end(); it != end;
             ++it) {
            layoutParameterEditor(it);
        }
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
}

void
EffectParameterDialog::layoutRenderTargetImage(
    sg_image handle, const Vector2UI16 &size, const String &name, const String &desc)
{
    ImGui::TextUnformatted(name.c_str());
    MutableString descMut(desc.c_str(), desc.c_str() + desc.size());
    descMut.push_back(0);
    ImGui::InputTextMultiline("##desc", descMut.data(), descMut.size(),
        ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 2), ImGuiInputTextFlags_ReadOnly);
    ImVec2 uv0, uv1;
    ImGuiWindow::getImageCoordinate(uv0, uv1);
    ImGui::Image(reinterpret_cast<ImTextureID>(handle.id), calcExpandedImageSize(size.x, size.y, 1.0f), uv0, uv1);
}

void
EffectParameterDialog::layoutParameterEditor(const effect::UIWidgetParameter *it)
{
    if (it->m_visible) {
        const String &name = it->m_name;
        ImGui::TextUnformatted(name.c_str());
        switch (it->m_widgetType) {
        case effect::UIWidgetParameter::kUIWidgetTypeCheckbox: {
            bool checked = it->m_valuePtr->data()[0].x != 0;
            if (ImGui::Checkbox(name.c_str(), &checked)) {
                it->m_valuePtr->data()[0].x = checked ? 1.0f : 0.0f;
            }
            break;
        }
        case effect::UIWidgetParameter::kUIWidgetTypeColor: {
            ImGui::ColorEdit3(name.c_str(), glm::value_ptr(it->m_valuePtr->data()[0]));
            break;
        }
        case effect::UIWidgetParameter::kUIWidgetTypeNumeric: {
            effect::ParameterType type = it->m_parameterType;
            Vector4 &values = it->m_valuePtr->data()[0];
            if (type >= effect::kParameterTypeFloat && type <= effect::kParameterTypeFloat4) {
                ImGui::InputFloat4(name.c_str(), glm::value_ptr(values));
            }
            else if (type >= effect::kParameterTypeInt && type <= effect::kParameterTypeInt4) {
                const glm::ivec2 range(it->range);
                Vector4SI32 v(values);
                ImGui::InputInt4(name.c_str(), glm::value_ptr(v));
                values = Vector4(v);
            }
            break;
        }
        case effect::UIWidgetParameter::kUIWidgetTypeSlider: {
            effect::ParameterType type = it->m_parameterType;
            Vector4 &values = it->m_valuePtr->data()[0];
            if (type >= effect::kParameterTypeFloat && type <= effect::kParameterTypeFloat4) {
                ImGui::SliderFloat4(name.c_str(), glm::value_ptr(values), it->range.x, it->range.y, "%.3f");
            }
            else if (type >= effect::kParameterTypeInt && type <= effect::kParameterTypeInt4) {
                const glm::ivec2 range(it->range);
                Vector4SI32 v(values);
                ImGui::SliderInt4(name.c_str(), glm::value_ptr(v), range.x, range.y, "%d");
                values = Vector4(v);
            }
            break;
        }
        case effect::UIWidgetParameter::kUIWidgetTypeSpinner: {
            const nanoem_f32_t step = 0.01f;
            effect::ParameterType type = it->m_parameterType;
            Vector4 &values = it->m_valuePtr->data()[0];
            if (type >= effect::kParameterTypeFloat && type <= effect::kParameterTypeFloat4) {
                ImGui::DragFloat4(name.c_str(), glm::value_ptr(values), it->range.x, it->range.y, step);
            }
            else if (type >= effect::kParameterTypeInt && type <= effect::kParameterTypeInt4) {
                const glm::ivec2 range(it->range);
                Vector4SI32 v(values);
                ImGui::DragInt4(name.c_str(), glm::value_ptr(v), range.x, range.y);
                values = Vector4(v);
            }
            break;
        }
        default:
            break;
        }
    }
}

void
EffectParameterDialog::layoutUniformBufferDetail(const String &name, const ByteArray &bytes)
{
    char label[Inline::kMarkerStringLength];
    Vector4 mut;
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(IM_COL32(0xff, 0xff, 0, 0xff)), "%s", name.c_str());
    switch (bytes.size()) {
    case 64: {
        const Matrix4x4 v(*reinterpret_cast<const Matrix4x4 *>(bytes.data()));
        for (int i = 0; i < 4; i++) {
            mut = v[i];
            StringUtils::format(label, sizeof(label), "##%s[%d]", name.c_str(), i);
            ImGui::InputFloat4(label, glm::value_ptr(mut), "%.6f", ImGuiInputTextFlags_ReadOnly);
        }
        break;
    }
    case 16: {
        const Vector4 *v = reinterpret_cast<const Vector4 *>(bytes.data());
        const Vector4::value_type e = glm::epsilon<Vector4::value_type>();
        mut = *v;
        StringUtils::format(label, sizeof(label), "##%s", name.c_str());
        if (glm::all(glm::epsilonEqual(*v, Vector4(v->x), e))) {
            ImGui::InputFloat(label, glm::value_ptr(mut), 0, 0, "%.6f", ImGuiInputTextFlags_ReadOnly);
        }
        else if (glm::epsilonEqual(v->z, 0.0f, e) && glm::epsilonEqual(v->w, 0.0f, e)) {
            ImGui::InputFloat2(label, glm::value_ptr(mut), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        else if (glm::epsilonEqual(v->w, 0.0f, e)) {
            ImGui::InputFloat3(label, glm::value_ptr(mut), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        else {
            ImGui::InputFloat4(label, glm::value_ptr(mut), "%.3f", ImGuiInputTextFlags_ReadOnly);
        }
        break;
    }
    default:
        if (bytes.size() % sizeof(Vector4) == 0) {
            nanoem_rsize_t numComponents = bytes.size() / sizeof(Vector4);
            const Vector4 *components = reinterpret_cast<const Vector4 *>(bytes.data());
            for (nanoem_rsize_t i = 0; i < numComponents; i++) {
                const Vector4 v(components[i]);
                ImGui::Text("[%ld] (%.6f, %.6f, %.6f, %.6f)", i, v.x, v.y, v.z, v.w);
            }
        }
        else if (bytes.size() % sizeof(nanoem_f32_t) == 0) {
            nanoem_rsize_t numComponents = bytes.size() / sizeof(nanoem_f32_t);
            const nanoem_f32_t *components = reinterpret_cast<const nanoem_f32_t *>(bytes.data());
            for (nanoem_rsize_t i = 0; i < numComponents; i++) {
                ImGui::Text("[%ld] %.6f", i, components[i]);
            }
        }
        break;
    }
}

void
EffectParameterDialog::queryFileDialog(IFileManager::DialogType type, Project *project)
{
    IEventPublisher *eventPublisher = project->eventPublisher();
    eventPublisher->publishQueryOpenSingleFileDialogEvent(type, Effect::loadableExtensions());
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
