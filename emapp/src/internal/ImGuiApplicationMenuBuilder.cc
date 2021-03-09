/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/ImGuiApplicationMenuBuilder.h"

#include "emapp/Accessory.h"
#include "emapp/BaseAudioPlayer.h"
#include "emapp/DefaultFileManager.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/Project.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"
#include "imgui/imgui.h"

#define SOKOL_GFX_INCLUDED /* stub */
#include "sokol/util/sokol_gfx_imgui.h"

namespace nanoem {
namespace internal {

struct ImGuiApplicationMenuBuilder::NewProjectEventHandler {
    static void
    saveProject(ImGuiApplicationMenuBuilder *self)
    {
        StringList extensions;
        extensions.push_back(Project::kArchivedNativeFormatFileExtension);
        extensions.push_back(Project::kPolygonMovieMakerFileExtension);
        extensions.push_back(Project::kFileSystemBasedNativeFormatFileExtension);
        self->m_eventPublisher->publishQuerySaveFileDialogEvent(IFileManager::kDialogTypeSaveProjectFile, extensions);
    }
    static void
    handleCompleteSavingFile(
        void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */, nanoem_u64_t /* ticks */)
    {
        ImGuiApplicationMenuBuilder *self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        self->m_client->sendNewProjectMessage();
    }
    static void
    handleSaveProjectProjectAfterConfirm(void *userData)
    {
        ImGuiApplicationMenuBuilder *self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        self->m_client->addCompleteSavingFileEventListener(handleCompleteSavingFile, self, true);
        saveProject(self);
    }
    static void
    handleDiscardProjectAfterConfirm(void *userData)
    {
        ImGuiApplicationMenuBuilder *self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        self->m_client->sendNewProjectMessage();
    }
    static void
    handleIsProjectDirtyResponse(void *userData, bool dirty)
    {
        ImGuiApplicationMenuBuilder *self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        BaseApplicationClient *client = self->m_client;
        if (dirty) {
            client->clearAllProjectAfterConfirmOnceEventListeners();
            client->addSaveProjectAfterConfirmEventListener(handleSaveProjectProjectAfterConfirm, self, true);
            client->addDiscardProjectAfterConfirmEventListener(handleDiscardProjectAfterConfirm, self, true);
            client->sendConfirmBeforeOpenProjectMessage();
        }
        else {
            client->sendNewProjectMessage();
        }
    }
};

struct ImGuiApplicationMenuBuilder::OpenProjectEventHandler {
    static void
    openProject(ImGuiApplicationMenuBuilder *self)
    {
        IEventPublisher *eventPublisher = self->m_eventPublisher;
        StringList extensions;
        extensions.push_back(Project::kArchivedNativeFormatFileExtension);
        extensions.push_back(Project::kPolygonMovieMakerFileExtension);
        extensions.push_back(Project::kFileSystemBasedNativeFormatFileExtension);
        eventPublisher->publishQueryOpenSingleFileDialogEvent(IFileManager::kDialogTypeOpenProject, extensions);
    }
    static void
    handleCompleteSavingFile(
        void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */, nanoem_u64_t /* ticks */)
    {
        auto self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        openProject(self);
    }
    static void
    handleSaveProjectAfterConfirm(void *userData)
    {
        auto self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        self->m_client->addCompleteSavingFileEventListener(handleCompleteSavingFile, self, true);
        ImGuiApplicationMenuBuilder::NewProjectEventHandler::saveProject(self);
    }
    static void
    handleDiscardProjectAfterConfirm(void *userData)
    {
        ImGuiApplicationMenuBuilder *self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        openProject(self);
    }
    static void
    handleIsProjectDirtyResponse(void *userData, bool dirty)
    {
        auto self = static_cast<ImGuiApplicationMenuBuilder *>(userData);
        BaseApplicationClient *client = self->m_client;
        if (dirty) {
            client->clearAllProjectAfterConfirmOnceEventListeners();
            client->addSaveProjectAfterConfirmEventListener(handleSaveProjectAfterConfirm, self, true);
            client->addDiscardProjectAfterConfirmEventListener(handleDiscardProjectAfterConfirm, self, true);
            client->sendConfirmBeforeOpenProjectMessage();
        }
        else {
            openProject(self);
        }
    }
};

ImGuiApplicationMenuBuilder::ImGuiApplicationMenuBuilder(BaseApplicationClient *client, IEventPublisher *eventPublisher,
    IFileManager *fileManager, const ITranslator *translator, bool enableModelEditing)
    : ApplicationMenuBuilder(client, enableModelEditing)
    , m_translator(translator)
    , m_client(client)
    , m_eventPublisher(eventPublisher)
    , m_fileManager(fileManager)
    , m_rootMenu(this)
    , m_rootMenuItem(&m_rootMenu)
{
}

ImGuiApplicationMenuBuilder::~ImGuiApplicationMenuBuilder()
{
    clearAllMenuItems();
}

void
ImGuiApplicationMenuBuilder::draw(void *debugger)
{
    if (ImGui::BeginMainMenuBar()) {
        m_rootMenuItem.draw();
        if (debugger) {
            if (ImGui::BeginMenu("Debug")) {
                sg_imgui_t *context = static_cast<sg_imgui_t *>(debugger);
                ImGui::MenuItem("Buffers", nullptr, &context->buffers.open);
                ImGui::MenuItem("Images", nullptr, &context->images.open);
                ImGui::MenuItem("Shaders", nullptr, &context->shaders.open);
                ImGui::MenuItem("Pipelines", nullptr, &context->pipelines.open);
                ImGui::MenuItem("Passes", nullptr, &context->passes.open);
                ImGui::MenuItem("Calls", nullptr, &context->capture.open);
                ImGui::EndMenu();
            }
        }
        ImGui::EndMainMenuBar();
    }
}

void
ImGuiApplicationMenuBuilder::createAllMenus()
{
    clearAllMenuItems();
    initialize();
    MainMenuBarHandle handle = reinterpret_cast<MainMenuBarHandle>(&m_rootMenuItem);
    createFileMenu(handle);
    createEditMenu(handle);
    appendMenuSeparator(m_editMenu);
    appendMenuItem(m_editMenu, kMenuItemTypeEditPreference);
    createProjectMenu(handle);
    createCameraMenu(handle);
    createLightMenu(handle);
    createModelMenu(handle);
    createAccessoryMenu(handle);
    createHelpMenu(handle);
}

ImGuiApplicationMenuBuilder::MenuBarHandle
ImGuiApplicationMenuBuilder::createMenuBar()
{
    ImGuiMenuBar *menu = nanoem_new(ImGuiMenuBar(this));
    m_objects.push_back(menu);
    return reinterpret_cast<MenuBarHandle>(menu);
}

ImGuiApplicationMenuBuilder::MenuBarHandle
ImGuiApplicationMenuBuilder::createMenuBar(MenuItemType type)
{
    ImGuiMenuBar *menu = nanoem_new(ImGuiMenuBar(this, type));
    m_objects.push_back(menu);
    return reinterpret_cast<MenuBarHandle>(menu);
}

ImGuiApplicationMenuBuilder::MenuItemHandle
ImGuiApplicationMenuBuilder::createMenuItem(MainMenuBarHandle menu)
{
    ImGuiMenuItem *parentItem = reinterpret_cast<ImGuiMenuItem *>(menu);
    ImGuiMenuBar *childMenuBar = nanoem_new(ImGuiMenuBar(this));
    m_objects.push_back(childMenuBar);
    ImGuiMenuItem *childMenuItem = nanoem_new(ImGuiMenuItem(childMenuBar));
    m_objects.push_back(childMenuItem);
    childMenuBar->m_items.push_back(childMenuItem);
    parentItem->m_children.push_back(childMenuBar);
    return reinterpret_cast<MenuItemHandle>(childMenuItem);
}

ImGuiApplicationMenuBuilder::MenuItemHandle
ImGuiApplicationMenuBuilder::appendMenuItem(MenuBarHandle menu, MenuItemType type)
{
    ImGuiMenuBar *parentMenuBar = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *childMenuItem = nanoem_new(ImGuiMenuItem(parentMenuBar, type));
    MenuItemHandle handle = reinterpret_cast<MenuItemHandle>(childMenuItem);
    m_objects.push_back(childMenuItem);
    m_menuItems.insert(tinystl::make_pair(type, handle));
    parentMenuBar->m_items.push_back(childMenuItem);
    return handle;
}

void
ImGuiApplicationMenuBuilder::createSelectAccessoryMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *item = nanoem_new(ImGuiMenuItem(parent, kMenuItemTypeAccessorySelectTitle));
    item->m_name = name;
    item->m_handle = handle;
    m_objects.push_back(item);
    parent->m_items.push_back(item);
}

void
ImGuiApplicationMenuBuilder::createSelectModelMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *item = nanoem_new(ImGuiMenuItem(parent, kMenuItemTypeModelSelectTitle));
    item->m_name = name;
    item->m_handle = handle;
    m_objects.push_back(item);
    parent->m_items.push_back(item);
}

void
ImGuiApplicationMenuBuilder::createSelectBoneMenuItem(MenuBarHandle menu, const char *name)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *item = nanoem_new(ImGuiMenuItem(parent, kMenuItemTypeModelSelectBoneTitle));
    item->m_name = name;
    m_objects.push_back(item);
    parent->m_items.push_back(item);
}

void
ImGuiApplicationMenuBuilder::createSelectMorphMenuItem(
    MenuBarHandle menu, nanoem_model_morph_category_t /* category */, const char *name)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *item = nanoem_new(ImGuiMenuItem(parent, kMenuItemTypeModelSelectMorphTitle));
    item->m_name = name;
    m_objects.push_back(item);
    parent->m_items.push_back(item);
}

void
ImGuiApplicationMenuBuilder::createPluginMenuItem(
    MenuBarHandle menu, MenuItemType type, nanoem_u16_t handle, const String &name, const StringList &items)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuBar *baseMenuBar = nanoem_new(ImGuiMenuBar(this));
    baseMenuBar->m_handle = handle;
    m_objects.push_back(baseMenuBar);
    ImGuiMenuItem *baseMenuBarItem = nanoem_new(ImGuiMenuItem(parent, type));
    m_objects.push_back(baseMenuBarItem);
    baseMenuBarItem->m_name = name;
    nanoem_u16_t functionIndex = 0;
    for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it) {
        ImGuiMenuItem *childMenuItem = nanoem_new(ImGuiMenuItem(baseMenuBar, type));
        childMenuItem->m_handle = functionIndex++;
        childMenuItem->m_name = *it;
        m_objects.push_back(childMenuItem);
        baseMenuBar->m_items.push_back(childMenuItem);
    }
    baseMenuBarItem->m_children.push_back(baseMenuBar);
    parent->m_items.push_back(baseMenuBarItem);
}

void
ImGuiApplicationMenuBuilder::updateAllSelectDrawableItems(MenuBarHandle menu, nanoem_u16_t handle)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    for (ImGuiMenuItemList::const_iterator it = parent->m_items.begin(), end = parent->m_items.end(); it != end; ++it) {
        ImGuiMenuItem *item = *it;
        item->m_checked = item->m_handle == handle;
    }
}

void
ImGuiApplicationMenuBuilder::removeMenuItemById(MenuBarHandle menu, int index)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    if (index >= 0 && index < Inline::saturateInt32(parent->m_items.size())) {
        parent->m_items.erase(parent->m_items.begin() + index);
    }
}

void
ImGuiApplicationMenuBuilder::appendMenuSeparator(MenuBarHandle menu)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    ImGuiMenuItem *item = nanoem_new(ImGuiMenuItem(parent));
    item->m_separator = true;
    m_objects.push_back(item);
    parent->m_items.push_back(item);
}

void
ImGuiApplicationMenuBuilder::clearAllMenuItems(MenuBarHandle menu)
{
    ImGuiMenuBar *parent = reinterpret_cast<ImGuiMenuBar *>(menu);
    parent->m_items.clear();
}

void
ImGuiApplicationMenuBuilder::setParentMenu(MenuItemHandle parent, MenuBarHandle menu)
{
    ImGuiMenuItem *parentItem = reinterpret_cast<ImGuiMenuItem *>(parent);
    ImGuiMenuBar *childMenu = reinterpret_cast<ImGuiMenuBar *>(menu);
    parentItem->m_children.push_back(childMenu);
}

void
ImGuiApplicationMenuBuilder::setMenuItemEnabled(MenuItemHandle item, bool value)
{
    ImGuiMenuItem *menuItem = reinterpret_cast<ImGuiMenuItem *>(item);
    menuItem->m_enabled = value;
}

void
ImGuiApplicationMenuBuilder::setMenuItemChecked(MenuItemHandle item, bool value)
{
    ImGuiMenuItem *menuItem = reinterpret_cast<ImGuiMenuItem *>(item);
    menuItem->m_checked = value;
}

bool
ImGuiApplicationMenuBuilder::isMenuItemEnabled(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT
{
    ImGuiMenuItem *menuItem = reinterpret_cast<ImGuiMenuItem *>(item);
    return menuItem->m_enabled;
}

bool
ImGuiApplicationMenuBuilder::isMenuItemChecked(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT
{
    ImGuiMenuItem *menuItem = reinterpret_cast<ImGuiMenuItem *>(item);
    return menuItem->m_checked;
}

void
ImGuiApplicationMenuBuilder::clearAllMenuItems()
{
    for (ImGuiMenuObjectList::const_iterator it = m_objects.begin(), end = m_objects.end(); it != end; ++it) {
        nanoem_delete(*it);
    }
    m_objects.clear();
    m_rootMenu.m_items.clear();
    m_rootMenuItem.m_children.clear();
}

void
ImGuiApplicationMenuBuilder::createHelpMenu(MainMenuBarHandle handle)
{
    MenuItemHandle helpMenu = createMenuItem(handle);
    m_helpMenu = createMenuBar(kMenuItemTypeHelpTitle);
    setParentMenu(helpMenu, m_helpMenu);
    appendMenuItem(m_helpMenu, kMenuItemTypeHelpAbout);
}

ImGuiApplicationMenuBuilder::ImGuiMenuItem::ImGuiMenuItem(ImGuiMenuBar *parent)
    : m_parent(parent)
    , m_type(kMenuItemTypeMaxEnum)
    , m_handle(bx::kInvalidHandle)
    , m_enabled(true)
    , m_checked(false)
    , m_separator(false)
{
}

ImGuiApplicationMenuBuilder::ImGuiMenuItem::ImGuiMenuItem(ImGuiMenuBar *parent, MenuItemType type)
    : m_parent(parent)
    , m_type(type)
    , m_handle(bx::kInvalidHandle)
    , m_enabled(true)
    , m_checked(false)
    , m_separator(false)
{
}

void
ImGuiApplicationMenuBuilder::ImGuiMenuItem::draw()
{
    if (!m_children.empty()) {
        if (m_type != kMenuItemTypeMaxEnum) {
            const char *text = nullptr;
            if (!m_name.empty()) {
                text = m_name.c_str();
            }
            else {
                ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
                const ITranslator *translator = builder->m_translator;
                text = translator->translate(menuItemString(m_type));
            }
            if (ImGui::BeginMenu(text, m_enabled)) {
                drawAllMenus();
                ImGui::EndMenu();
            }
        }
        else {
            drawAllMenus();
        }
    }
    else if (m_separator) {
        ImGui::Separator();
    }
    else if (m_type != kMenuItemTypeMaxEnum) {
        ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
        switch (m_type) {
        case kMenuItemTypeModelSelectTitle: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendSetActiveModelMessage(m_handle);
            }
            break;
        }
        case kMenuItemTypeAccessorySelectTitle: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendSetActiveAccessoryMessage(m_handle);
            }
            break;
        }
        case kMenuItemTypeModelSelectBoneTitle: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendSetActiveModelBoneMessage(m_name.c_str());
            }
            break;
        }
        case kMenuItemTypeModelSelectMorphTitle: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendSetActiveModelMorphMessage(m_name.c_str(), true);
            }
            break;
        }
        case kMenuItemTypeModelPluginExecute: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendExecuteModelPluginMessage(m_parent->m_handle, m_handle);
            }
            break;
        }
        case kMenuItemTypeMotionPluginExecute: {
            if (ImGui::MenuItem(m_name.c_str(), nullptr, m_checked, m_enabled)) {
                BaseApplicationClient *client = m_parent->m_parent->m_client;
                client->sendExecuteMotionPluginMessage(m_parent->m_handle, m_handle);
            }
            break;
        }
        default: {
            const ITranslator *translator = builder->m_translator;
            const char *text = translator->translate(menuItemString(m_type));
            if (ImGui::MenuItem(text, nullptr, m_checked, m_enabled)) {
                switch (m_type) {
                case kMenuItemTypeFileNewProject: {
                    ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
                    BaseApplicationClient *client = builder->m_client;
                    client->sendIsProjectDirtyRequestMessage(
                        NewProjectEventHandler::handleIsProjectDirtyResponse, builder);
                    break;
                }
                case kMenuItemTypeFileOpenProject: {
                    ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
                    BaseApplicationClient *client = builder->m_client;
                    client->sendIsProjectDirtyRequestMessage(
                        OpenProjectEventHandler::handleIsProjectDirtyResponse, builder);
                    break;
                }
                case kMenuItemTypeFileImportAudioSource: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenAudioFile, BaseAudioPlayer::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportAccessory: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenModelFile, Accessory::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportModel: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenModelFile, Model::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportModelPose: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenModelMotionFile, model::BindPose::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportModelMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenModelMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportLightMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenLightMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileImportCameraMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQueryOpenSingleFileDialogEvent(
                        IFileManager::kDialogTypeOpenCameraMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileSaveProject:
                case kMenuItemTypeFileSaveAsProject: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    StringList extensions;
                    extensions.push_back(Project::kArchivedNativeFormatFileExtension);
                    extensions.push_back(Project::kFileSystemBasedNativeFormatFileExtension);
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveProjectFile, extensions);
                    break;
                }
                case kMenuItemTypeFileExportModelPose: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveModelMotionFile, model::BindPose::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileExportModel: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveModelFile, Model::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileExportModelMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveModelMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileExportLightMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveLightMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileExportCameraMotion: {
                    IEventPublisher *eventPublisher = builder->m_eventPublisher;
                    eventPublisher->publishQuerySaveFileDialogEvent(
                        IFileManager::kDialogTypeSaveCameraMotionFile, Motion::loadableExtensions());
                    break;
                }
                case kMenuItemTypeFileExportImage: {
                    struct ExportImageCallbackHandler {
                        static void
                        handleTransientQueryFileDialog(const URI &fileURI, Project * /* project */, void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->sendExecuteExportingImageMessage(fileURI);
                        }
                        static void
                        handleCompleteExportImageConfiguration(void *userData, const StringList &availableExtensions)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            if (!availableExtensions.empty()) {
                                builder->m_fileManager->setTransientQueryFileDialogCallback(
                                    handleTransientQueryFileDialog, builder);
                                builder->m_eventPublisher->publishQuerySaveFileDialogEvent(
                                    IFileManager::kDialogTypeUserCallback, availableExtensions);
                            }
                            else {
                                builder->m_client->sendExecuteExportingImageMessage(URI());
                            }
                        }
                        static void
                        handleSaveProjectAfterConfirm(void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->addCompleteSavingFileEventListener(
                                handleCompleteSavingFile, builder, true);
                            NewProjectEventHandler::saveProject(builder);
                        }
                        static void
                        handleCompleteSavingFile(void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */,
                            nanoem_u64_t /* ticks */)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            handleDiscardProjectAfterConfirm(builder);
                        }
                        static void
                        handleDiscardProjectAfterConfirm(void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->addCompleteExportImageConfigurationEventListener(
                                handleCompleteExportImageConfiguration, builder, true);
                            builder->m_client->sendRequestExportImageConfigurationMessage();
                        }
                    };
                    ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
                    BaseApplicationClient *client = builder->m_client;
                    client->addSaveProjectAfterConfirmEventListener(
                        ExportImageCallbackHandler::handleSaveProjectAfterConfirm, builder, true);
                    client->addDiscardProjectAfterConfirmEventListener(
                        ExportImageCallbackHandler::handleDiscardProjectAfterConfirm, builder, true);
                    client->sendConfirmBeforeExportingImageMessage();
                    break;
                }
                case kMenuItemTypeFileExportVideo: {
                    struct ExportVideoCallbackHandler {
                        static void
                        handleTransientQueryFileDialog(const URI &fileURI, Project * /* project */, void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->sendExecuteExportingVideoMessage(fileURI);
                        }
                        static void
                        handleCompleteExportVideoConfiguration(void *userData, const StringList &availableExtensions)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            if (!availableExtensions.empty()) {
                                builder->m_fileManager->setTransientQueryFileDialogCallback(
                                    handleTransientQueryFileDialog, builder);
                                builder->m_eventPublisher->publishQuerySaveFileDialogEvent(
                                    IFileManager::kDialogTypeUserCallback, availableExtensions);
                            }
                            else {
                                builder->m_client->sendExecuteExportingVideoMessage(URI());
                            }
                        }
                        static void
                        handleSaveProjectAfterConfirm(void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->addCompleteSavingFileEventListener(
                                handleCompleteSavingFile, builder, true);
                            NewProjectEventHandler::saveProject(builder);
                        }
                        static void
                        handleCompleteSavingFile(void *userData, const URI & /* fileURI */, nanoem_u32_t /* type */,
                            nanoem_u64_t /* ticks */)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            handleDiscardProjectAfterConfirm(builder);
                        }
                        static void
                        handleDiscardProjectAfterConfirm(void *userData)
                        {
                            ImGuiApplicationMenuBuilder *builder = static_cast<ImGuiApplicationMenuBuilder *>(userData);
                            builder->m_client->addCompleteExportVideoConfigurationEventListener(
                                handleCompleteExportVideoConfiguration, builder, true);
                            builder->m_client->sendRequestExportVideoConfigurationMessage();
                        }
                    };
                    ImGuiApplicationMenuBuilder *builder = m_parent->m_parent;
                    BaseApplicationClient *client = builder->m_client;
                    client->addSaveProjectAfterConfirmEventListener(
                        ExportVideoCallbackHandler::handleSaveProjectAfterConfirm, builder, true);
                    client->addDiscardProjectAfterConfirmEventListener(
                        ExportVideoCallbackHandler::handleDiscardProjectAfterConfirm, builder, true);
                    client->sendConfirmBeforeExportingVideoMessage();
                    break;
                }
                default:
                    builder->m_client->sendMenuActionMessage(m_type);
                    break;
                }
            }
            break;
        }
        }
    }
}

void
ImGuiApplicationMenuBuilder::ImGuiMenuItem::drawAllMenus()
{
    for (ImGuiMenuBarList::const_iterator it = m_children.begin(), end = m_children.end(); it != end; ++it) {
        ImGuiMenuBar *menu = *it;
        menu->draw();
    }
}

ImGuiApplicationMenuBuilder::ImGuiMenuBar::ImGuiMenuBar(ImGuiApplicationMenuBuilder *parent)
    : m_parent(parent)
    , m_type(kMenuItemTypeMaxEnum)
    , m_handle(0)
{
}

ImGuiApplicationMenuBuilder::ImGuiMenuBar::ImGuiMenuBar(ImGuiApplicationMenuBuilder *parent, MenuItemType type)
    : m_parent(parent)
    , m_type(type)
    , m_handle(0)
{
}

void
ImGuiApplicationMenuBuilder::ImGuiMenuBar::draw()
{
    if (m_type != kMenuItemTypeMaxEnum) {
        const ITranslator *translator = m_parent->m_translator;
        const char *text = translator->translate(menuItemString(m_type));
        if (ImGui::BeginMenu(text)) {
            drawAllMenuItems();
            ImGui::EndMenu();
        }
    }
    else {
        drawAllMenuItems();
    }
}

void
ImGuiApplicationMenuBuilder::ImGuiMenuBar::drawAllMenuItems()
{
    for (ImGuiMenuItemList::const_iterator it = m_items.begin(), end = m_items.end(); it != end; ++it) {
        ImGuiMenuItem *item = *it;
        item->draw();
    }
}

} /* namespace internal */
} /* namespace nanoem */
