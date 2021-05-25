/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/PreferenceDialog.h"

#include "emapp/Error.h"
#include "emapp/Grid.h"
#include "emapp/private/CommonInclude.h"

#include "undo/undo.h"

namespace nanoem {
namespace internal {
namespace imgui {

const char *const PreferenceDialog::kIdentifier = "dialog.preference";

PreferenceDialog::PreferenceDialog(BaseApplicationService *applicationPtr, ImGuiWindow *parent)
    : BaseNonModalDialogWindow(applicationPtr)
    , m_parent(parent)
{
}

bool
PreferenceDialog::draw(Project *project)
{
    bool visible = true;
    if (open(
            tr("nanoem.gui.window.preference.title"), kIdentifier, &visible, ImGui::GetFrameHeightWithSpacing() * 21)) {
        ImGui::BeginTabBar("tabbar");
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.preference.tab.global"))) {
            ImGui::PushItemWidth(-1);
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.global.renderer"));
            ApplicationPreference preference(application());
            const StringList renderers(preference.allAvailableRenderers());
            if (ImGui::BeginCombo("##renderer", preference.rendererBackend())) {
                for (StringList::const_iterator it = renderers.begin(), end = renderers.end(); it != end; ++it) {
                    const char *name = it->c_str();
                    if (ImGui::Selectable(name)) {
                        preference.setRendererBackend(name);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.global.color-format"));
            if (ImGui::BeginCombo("##color-format", selectedPixelFormatString(preference.defaultColorPixelFormat()))) {
                static const sg_pixel_format kPreferredColorPixelFormatMenuItems[] = {
                    SG_PIXELFORMAT_RGBA8,
                    SG_PIXELFORMAT_RGB10A2,
                    SG_PIXELFORMAT_RGBA16F,
                };
                for (size_t i = 0; i < BX_COUNTOF(kPreferredColorPixelFormatMenuItems); i++) {
                    const sg_pixel_format menuItemValue = kPreferredColorPixelFormatMenuItems[i];
                    if (ImGui::Selectable(selectedPixelFormatString(menuItemValue))) {
                        preference.setDefaultColorPixelFormat(menuItemValue);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.global.fps"));
            if (ImGui::BeginCombo("##framerate", selectedFPSMenuItemString(project->preferredEditingFPS()))) {
                static const nanoem_u32_t kPreferredFPSMenuItems[] = { 0, 30, 60, UINT32_MAX };
                for (size_t i = 0; i < BX_COUNTOF(kPreferredFPSMenuItems); i++) {
                    const nanoem_u32_t menuItemValue = kPreferredFPSMenuItems[i];
                    if (ImGui::Selectable(selectedFPSMenuItemString(menuItemValue))) {
                        project->setPreferredEditingFPS(menuItemValue);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.viewport.high-dpi.mode"));
            if (ImGui::BeginCombo(
                    "##viewport-mode", selectedHighDPIViewportModeString(preference.highDPIViewportMode()))) {
                for (int i = ApplicationPreference::kHighDPIViewportModeFirstEnum;
                     i < ApplicationPreference::kHighDPIViewportModeMaxEnum; i++) {
                    const ApplicationPreference::HighDPIViewportModeType menuItemValue =
                        static_cast<ApplicationPreference::HighDPIViewportModeType>(i);
                    if (ImGui::Selectable(selectedHighDPIViewportModeString(menuItemValue))) {
                        preference.setHighDPIViewportMode(menuItemValue);
                    }
                }
                ImGui::EndCombo();
            }
            addSeparator();
            bool enableModelEditing = preference.isModelEditingEnabled();
            if (ImGui::Checkbox(tr("nanoem.gui.window.preference.global.editing-model.enable"), &enableModelEditing)) {
                preference.setModelEditingEnabled(enableModelEditing);
            }
            bool enableSkinDeformerAccelerator = preference.isSkinDeformAcceleratorEnabled();
            if (ImGui::Checkbox(tr("nanoem.gui.window.preference.global.sda.enable"), &enableSkinDeformerAccelerator)) {
                preference.setSkinDeformAcceleratorEnabled(enableSkinDeformerAccelerator);
            }
            addSeparator();
            bool enableCrashReport = preference.isCrashReportEnabled();
            if (ImGui::Checkbox(tr("nanoem.gui.window.preference.global.crash-report.enable"), &enableCrashReport)) {
                preference.setCrashReportEnabled(enableCrashReport);
            }
            bool enableTracking = preference.isAnalyticsEnabled();
            if (ImGui::Checkbox(tr("nanoem.gui.window.preference.global.analytics.enable"), &enableTracking)) {
                preference.setAnalyticsEnabled(enableTracking);
            }
            if (m_parent->handleTranslatedButton("nanoem.gui.window.preference.global.analytics.reset")) {
                preference.setResettingAnalyticsUUIDRequired(true);
            }
            ImGui::PopItemWidth();
            addSeparator();
            if (ImGui::TreeNode(tr("nanoem.gui.window.preference.global.system-information"))) {
                ImGui::Text("CPU: %s (%s)", BX_CPU_NAME, BX_ARCH_NAME);
                ImGui::Text("Platform: %s", BX_PLATFORM_NAME);
                ImGui::Text("Compiler: %s (%s)", BX_COMPILER_NAME, BX_CPP_NAME);
                ImGui::Text("CRT: %s", BX_CRT_NAME);
                ImGui::TreePop();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem(tr("nanoem.gui.window.preference.tab.project"))) {
            ImGui::PushItemWidth(-1);
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.project.language.title"));
            ITranslator *translator = application()->translator();
            const ITranslator::LanguageType language = translator->language();
            if (ImGui::BeginCombo("##language", selectedLanguageString(language))) {
                const nanoem_u32_t flags = project->isModelEditingEnabled() ? ImGuiSelectableFlags_Disabled : 0;
                for (int i = ITranslator::kLanguageTypeFirstEnum; i < ITranslator::kLanguageTypeMaxEnum; i++) {
                    ITranslator::LanguageType type = static_cast<ITranslator::LanguageType>(i);
                    if (translator->isSupportedLanguage(type) &&
                        ImGui::Selectable(selectedLanguageString(type), type == language, flags)) {
                        project->setLanguage(type);
                        translator->setLanguage(type);
                    }
                }
                ImGui::EndCombo();
            }
            addSeparator();
            static const ApplicationMenuBuilder::MenuItemType itemTypes[] = {
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableGrid,
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableGroundShadow,
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableEffect,
            };
            char buffer[Inline::kNameStackBufferSize];
            BaseApplicationService *app = application();
            for (nanoem_rsize_t i = 0; i < BX_COUNTOF(itemTypes); i++) {
                ApplicationMenuBuilder::MenuItemType itemType = itemTypes[i];
                ApplicationMenuBuilder::MenuItemCheckedState state;
                ApplicationMenuBuilder::validateMenuItem(project, itemType, state);
                bool checked = state == ApplicationMenuBuilder::kMenuItemCheckedStateTrue;
                ApplicationMenuBuilder::stripMnemonic(buffer, sizeof(buffer), app->translateMenuItem(itemType));
                if (ImGui::Checkbox(buffer, &checked)) {
                    Error error;
                    app->dispatchMenuItemAction(project, itemType, error);
                    error.notify(app->eventPublisher());
                }
            }
            addSeparator();
            Grid *grid = project->grid();
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.grid.title"));
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.grid.size.cell"));
            Vector2 gridSize(grid->size());
            if (ImGui::DragFloat2("##grid.size", glm::value_ptr(gridSize), 0.01f, 0.01f, FLT_MAX)) {
                grid->resize(gridSize);
            }
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.grid.size.grid"));
            Vector2SI32 numCells(grid->cell());
            if (ImGui::DragInt2("##grid.cell", glm::value_ptr(numCells), 0.05f, 1, 0xff)) {
                grid->setCell(numCells);
            }
            addSeparator();
            ImGui::TextUnformatted(app->translateMenuItem(ApplicationMenuBuilder::kMenuItemTypeProjectMSAATitle));
            static const ApplicationMenuBuilder::MenuItemType msaaItemTypes[] = {
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx16,
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx8,
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx4,
                ApplicationMenuBuilder::kMenuItemTypeProjectEnableMSAAx2,
                ApplicationMenuBuilder::kMenuItemTypeProjectDisableMSAA,
            };
            if (ImGui::BeginCombo(
                    "##msaa", selectedMenuItemString(project, msaaItemTypes, BX_COUNTOF(msaaItemTypes)))) {
                layoutPreferenceMenuItemCombo(project, msaaItemTypes, BX_COUNTOF(msaaItemTypes));
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(app->translateMenuItem(
                ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationTitle));
            static const ApplicationMenuBuilder::MenuItemType physicsSimulationModeTypes[] = {
                ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableAnytime,
                ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnablePlaying,
                ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationEnableTracing,
                ApplicationMenuBuilder::kMenuItemTypeProjectPhysicsSimulationDisable
            };
            if (ImGui::BeginCombo("##simulation",
                    selectedMenuItemString(
                        project, physicsSimulationModeTypes, BX_COUNTOF(physicsSimulationModeTypes)))) {
                layoutPreferenceMenuItemCombo(
                    project, physicsSimulationModeTypes, BX_COUNTOF(physicsSimulationModeTypes));
                ImGui::EndCombo();
            }
            static const ApplicationMenuBuilder::MenuItemType preferredFPS[] = {
                ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPSUnlimited,
                ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS60,
                ApplicationMenuBuilder::kMenuItemTypeProjectPreferredMotionFPS30
            };
            ImGui::TextUnformatted(
                app->translateMenuItem(ApplicationMenuBuilder::kMenuItemTypeProjectPreferredFPSTitle));
            if (ImGui::BeginCombo(
                    "##framerate", selectedMenuItemString(project, preferredFPS, BX_COUNTOF(preferredFPS)))) {
                layoutPreferenceMenuItemCombo(project, preferredFPS, BX_COUNTOF(preferredFPS));
                ImGui::EndCombo();
            }
            ImGui::TextUnformatted(tr("nanoem.gui.window.preference.file-path-mode.title"));
            if (ImGui::BeginCombo("##file-path-mode", selectedFilePathModeMenuItemString(project->filePathMode()))) {
                for (size_t i = Project::kFilePathModeFirstEnum; i < Project::kFilePathModeMaxEnum; i++) {
                    const Project::FilePathMode mode = static_cast<Project::FilePathMode>(i);
                    if (ImGui::Selectable(selectedFilePathModeMenuItemString(mode))) {
                        project->setFilePathMode(mode);
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::PopItemWidth();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Special")) {
            ImGui::PushItemWidth(-1);
            ApplicationPreference preference(application());
            ImGui::TextUnformatted("Application Preference");
            if (ImGui::Button("Initialize##preferences.reset", ImVec2(-1, 0))) {
                preference.setUndoSoftLimit(ApplicationPreference::kUndoSoftLimitDefaultValue);
                preference.setGFXBufferPoolSize(ApplicationPreference::kGFXBufferPoolSizeDefaultValue);
                preference.setGFXImagePoolSize(ApplicationPreference::kGFXImagePoolSizeDefaultValue);
                preference.setGFXShaderPoolSize(ApplicationPreference::kGFXShaderPoolSizeDefaultValue);
                preference.setGFXPassPoolSize(ApplicationPreference::kGFXPassPoolSizeDefaultValue);
                preference.setGFXPipelinePoolSize(ApplicationPreference::kGFXPipelinePoolSizeDefaultValue);
                preference.setGFXUniformBufferSize(ApplicationPreference::kGFXUniformBufferSizeDefaultValue);
            }
            addSeparator();
            {
                int value = preference.undoSoftLimit();
                ImGui::TextUnformatted("Undo Soft Limit");
                if (ImGui::DragInt("##preference.undo", &value, 1.0f, 64, undoStackGetHardLimit())) {
                    preference.setUndoSoftLimit(value);
                }
            }
            addSeparator();
            {
                int value = preference.gfxBufferPoolSize();
                ImGui::TextUnformatted("Buffer Pool Size");
                if (ImGui::DragInt("##preference.gfx.pool.buffer", &value, 1.0f, 1024, 0xffff)) {
                    preference.setGFXBufferPoolSize(value);
                }
            }
            {
                int value = preference.gfxImagePoolSize();
                ImGui::TextUnformatted("Image Pool Size");
                if (ImGui::DragInt("##preference.gfx.pool.image", &value, 1.0f, 2048, 0xffff)) {
                    preference.setGFXImagePoolSize(value);
                }
            }
            {
                int value = preference.gfxShaderPoolSize();
                ImGui::TextUnformatted("Shader Pool Size");
                if (ImGui::DragInt("##preference.gfx.pool.shader", &value, 1.0f, 1024, 0xffff)) {
                    preference.setGFXShaderPoolSize(value);
                }
            }
            {
                int value = preference.gfxPassPoolSize();
                ImGui::TextUnformatted("Pass Pool Size");
                if (ImGui::DragInt("##preference.gfx.pool.pass", &value, 1.0f, 512, 0xffff)) {
                    preference.setGFXPassPoolSize(value);
                }
            }
            {
                int value = preference.gfxPipelinePoolSize();
                ImGui::TextUnformatted("Pipeline Pool Size");
                if (ImGui::DragInt("##preference.gfx.pool.pipeline", &value, 1.0f, 1024, 0xffff)) {
                    preference.setGFXPipelinePoolSize(value);
                }
            }
            {
                int value = preference.gfxUniformBufferSize();
                ImGui::TextUnformatted("Uniform Buffer Size");
                if (ImGui::DragInt("##preference.gfx.buffer.uniform", &value, 1.0f, 0x10000, 0x7fffff)) {
                    preference.setGFXUniformBufferSize(value);
                }
            }
            ImGui::PopItemWidth();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    close();
    return visible;
}

const char *
PreferenceDialog::selectedMenuItemString(Project *project, const ApplicationMenuBuilder::MenuItemType *items,
    nanoem_rsize_t numItems) const NANOEM_DECL_NOEXCEPT
{
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        ApplicationMenuBuilder::MenuItemType itemType = items[i];
        ApplicationMenuBuilder::MenuItemCheckedState state;
        ApplicationMenuBuilder::validateMenuItem(project, itemType, state);
        if (state == ApplicationMenuBuilder::kMenuItemCheckedStateTrue) {
            return application()->translateMenuItem(itemType);
        }
    }
    return "(Unknown)";
}

void
PreferenceDialog::layoutPreferenceMenuItemCombo(
    Project *project, const ApplicationMenuBuilder::MenuItemType *items, nanoem_rsize_t numItems)
{
    BaseApplicationService *app = application();
    for (nanoem_rsize_t i = 0; i < numItems; i++) {
        ApplicationMenuBuilder::MenuItemType itemType = items[i];
        ApplicationMenuBuilder::MenuItemCheckedState state;
        ApplicationMenuBuilder::validateMenuItem(project, itemType, state);
        if (ImGui::Selectable(app->translateMenuItem(itemType))) {
            Error error;
            app->dispatchMenuItemAction(project, itemType, error);
            error.notify(project->eventPublisher());
        }
    }
}

const char *
PreferenceDialog::selectedPixelFormatString(sg_pixel_format value) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = application()->translator();
    switch (value) {
    case SG_PIXELFORMAT_RGBA8:
    case _SG_PIXELFORMAT_DEFAULT:
        return translator->translate("nanoem.gui.window.preference.global.color-format.rgba8");
    case SG_PIXELFORMAT_RGB10A2:
        return translator->translate("nanoem.gui.window.preference.global.color-format.rgb10a2");
    case SG_PIXELFORMAT_RGBA16F:
        return translator->translate("nanoem.gui.window.preference.global.color-format.rgba16f");
    default:
        return "(Unknown)";
    }
}

const char *
PreferenceDialog::selectedLanguageString(ITranslator::LanguageType value) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = application()->translator();
    switch (value) {
    case ITranslator::kLanguageTypeEnglish:
        return translator->translate("nanoem.gui.window.preference.project.language.english");
    case ITranslator::kLanguageTypeJapanese:
        return translator->translate("nanoem.gui.window.preference.project.language.japanese");
    default:
        return "(Unknown)";
    }
}

const char *
PreferenceDialog::selectedFPSMenuItemString(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = application()->translator();
    switch (value) {
    case UINT32_MAX:
        return translator->translate("nanoem.gui.window.preference.global.fps.unlimited");
    case 60:
        return translator->translate("nanoem.gui.window.preference.global.fps.60");
    case 30:
        return translator->translate("nanoem.gui.window.preference.global.fps.30");
    case 0:
        return translator->translate("nanoem.gui.window.preference.global.fps.auto");
    default:
        return "(Unknown)";
    }
}

const char *
PreferenceDialog::selectedHighDPIViewportModeString(
    ApplicationPreference::HighDPIViewportModeType value) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = application()->translator();
    switch (value) {
    case ApplicationPreference::kHighDPIViewportModeAuto:
        return translator->translate("nanoem.gui.window.preference.viewport.high-dpi.mode.auto");
    case ApplicationPreference::kHighDPIViewportModeEnabled:
        return translator->translate("nanoem.gui.window.preference.viewport.high-dpi.mode.enabled");
    case ApplicationPreference::kHighDPIViewportModeDisabled:
        return translator->translate("nanoem.gui.window.preference.viewport.high-dpi.mode.disabled");
    default:
        return "(Unknown)";
    }
}

const char *
PreferenceDialog::selectedFilePathModeMenuItemString(Project::FilePathMode value) const NANOEM_DECL_NOEXCEPT
{
    const ITranslator *translator = application()->translator();
    switch (value) {
    case Project::kFilePathModeAbsolute:
        return translator->translate("nanoem.gui.window.preference.file-path-mode.absolute");
    case Project::kFilePathModeRelative:
        return translator->translate("nanoem.gui.window.preference.file-path-mode.relative");
    default:
        return "(Unknown)";
    }
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
