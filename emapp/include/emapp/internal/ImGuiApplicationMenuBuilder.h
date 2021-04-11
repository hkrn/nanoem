/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUIAPPLICATIONMENUBUILDER_H_
#define NANOEM_EMAPP_INTERNAL_IMGUIAPPLICATIONMENUBUILDER_H_

#include "emapp/ApplicationMenuBuilder.h"

#include "emapp/ThreadedApplicationClient.h"

namespace nanoem {

class IEventPublisher;
class IFileManager;
class ITranslator;

namespace internal {

class DraggingMorphSliderState;

class ImGuiApplicationMenuBuilder NANOEM_DECL_SEALED : public ApplicationMenuBuilder {
public:
    ImGuiApplicationMenuBuilder(BaseApplicationClient *client, IEventPublisher *eventPublisher,
        IFileManager *fileManager, const ITranslator *translator, bool enableModelEditing);
    ~ImGuiApplicationMenuBuilder();

    void draw(void *debugger);

private:
    struct NewProjectEventHandler;
    struct OpenProjectEventHandler;
    struct ExportImageCallbackHandler;
    struct ExportVideoCallbackHandler;

    void createAllMenus() NANOEM_DECL_OVERRIDE;
    MenuBarHandle createMenuBar() NANOEM_DECL_OVERRIDE;
    MenuBarHandle createMenuBar(MenuItemType type) NANOEM_DECL_OVERRIDE;
    MenuItemHandle createMenuItem(MainMenuBarHandle menu) NANOEM_DECL_OVERRIDE;
    MenuItemHandle appendMenuItem(MenuBarHandle menu, MenuItemType type) NANOEM_DECL_OVERRIDE;
    void createSelectAccessoryMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name) NANOEM_DECL_OVERRIDE;
    void createSelectModelMenuItem(MenuBarHandle menu, nanoem_u16_t handle, const char *name) NANOEM_DECL_OVERRIDE;
    void createSelectBoneMenuItem(MenuBarHandle menu, const char *name) NANOEM_DECL_OVERRIDE;
    void createSelectMorphMenuItem(
        MenuBarHandle menu, nanoem_model_morph_category_t category, const char *name) NANOEM_DECL_OVERRIDE;
    void createPluginMenuItem(MenuBarHandle menu, MenuItemType type, nanoem_u16_t handle, const String &name,
        const StringList &items) NANOEM_DECL_OVERRIDE;
    void updateAllSelectDrawableItems(MenuBarHandle menu, nanoem_u16_t handle) NANOEM_DECL_OVERRIDE;
    void removeMenuItemById(MenuBarHandle menu, int index) NANOEM_DECL_OVERRIDE;

    void appendMenuSeparator(MenuBarHandle menu) NANOEM_DECL_OVERRIDE;
    void clearAllMenuItems(MenuBarHandle menu) NANOEM_DECL_OVERRIDE;
    void setParentMenu(MenuItemHandle parent, MenuBarHandle menu) NANOEM_DECL_OVERRIDE;
    void setMenuItemEnabled(MenuItemHandle item, bool value) NANOEM_DECL_OVERRIDE;
    void setMenuItemChecked(MenuItemHandle item, bool value) NANOEM_DECL_OVERRIDE;
    bool isMenuItemEnabled(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isMenuItemChecked(MenuItemHandle item) const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    void clearAllMenuItems();
    void createHelpMenu(MainMenuBarHandle handle);

    struct ImGuiMenuBar;
    struct ImGuiMenuItem;
    typedef tinystl::vector<ImGuiMenuBar *, TinySTLAllocator> ImGuiMenuBarList;
    typedef tinystl::vector<ImGuiMenuItem *, TinySTLAllocator> ImGuiMenuItemList;
    struct ImGuiMenuObject {
        virtual ~ImGuiMenuObject() NANOEM_DECL_NOEXCEPT
        {
        }
        virtual void draw() = 0;
    };
    struct ImGuiMenuItem : ImGuiMenuObject {
        ImGuiMenuItem(ImGuiMenuBar *parent);
        ImGuiMenuItem(ImGuiMenuBar *parent, MenuItemType type);
        void draw() NANOEM_DECL_OVERRIDE;
        void drawAllMenus();
        ImGuiMenuBar *m_parent;
        ImGuiMenuBarList m_children;
        MenuItemType m_type;
        String m_name;
        nanoem_u16_t m_handle;
        bool m_enabled;
        bool m_checked;
        bool m_separator;
    };
    struct ImGuiMenuBar : ImGuiMenuObject {
        ImGuiMenuBar(ImGuiApplicationMenuBuilder *parent);
        ImGuiMenuBar(ImGuiApplicationMenuBuilder *parent, MenuItemType type);
        void draw() NANOEM_DECL_OVERRIDE;
        void drawAllMenuItems();
        ImGuiApplicationMenuBuilder *m_parent;
        ImGuiMenuItemList m_items;
        MenuItemType m_type;
        nanoem_u16_t m_handle;
    };
    struct FileDialogState {
        FileDialogState();
        ~FileDialogState() NANOEM_DECL_NOEXCEPT;

        virtual void execute(const URI &fileURI, BaseApplicationClient *client) = 0;
        virtual const char *windowID() const NANOEM_DECL_NOEXCEPT = 0;
        virtual const char *windowTitle() const NANOEM_DECL_NOEXCEPT = 0;

        void initialize(const StringList &extensions, nanoem_u32_t type);
        void draw(BaseApplicationClient *client);
        bool hasAllowedExtensions() const NANOEM_DECL_NOEXCEPT;

        void *m_instance;
        StringList m_allowedExtensions;
        nanoem_u32_t m_type;
        bool m_opened;
    };
    struct OpenFileDialogState : FileDialogState {
        void execute(const URI &fileURI, BaseApplicationClient *client) NANOEM_DECL_OVERRIDE;
        const char *windowID() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
        const char *windowTitle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    };
    struct SaveFileDialogState : FileDialogState {
        void execute(const URI &fileURI, BaseApplicationClient *client) NANOEM_DECL_OVERRIDE;
        const char *windowID() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
        const char *windowTitle() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    };

    typedef tinystl::vector<ImGuiMenuObject *, TinySTLAllocator> ImGuiMenuObjectList;
    const ITranslator *m_translator;
    BaseApplicationClient *m_client;
    IEventPublisher *m_eventPublisher;
    IFileManager *m_fileManager;
    ImGuiMenuObjectList m_objects;
    ImGuiMenuBar m_rootMenu;
    ImGuiMenuItem m_rootMenuItem;
    ImGuiMenuBarList m_menus;
    OpenFileDialogState m_openFileDialogState;
    SaveFileDialogState m_saveFileDialogState;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUIAPPLICATIONMENUBUILDER_H_ */
