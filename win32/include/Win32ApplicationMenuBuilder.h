/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_WIN32APPLICATIONMENUBUILDER_H_
#define NANOEM_EMAPP_WIN32_WIN32APPLICATIONMENUBUILDER_H_

#include "emapp/ApplicationMenuBuilder.h"

#include <functional>
#include <unordered_map>
#include <windows.h>

namespace bx {
class HandleAlloc;
}

namespace nanoem {

class ITranslator;
class ThreadedApplicationClient;

namespace win32 {

class MainWindow;

class Win32ApplicationMenuBuilder final : public ApplicationMenuBuilder {
public:
    Win32ApplicationMenuBuilder(
        MainWindow *window, ThreadedApplicationClient *client, const ITranslator *translator, bool enableModelEditing);
    ~Win32ApplicationMenuBuilder() noexcept override;

    void dispatch(UINT menuItemID);

private:
    using Lambda = std::function<void()>;
    struct Win32Menu {
        HMENU parentMenu;
        MenuItemType type;
    };
    struct Win32MenuItem {
        HMENU parentMenu;
        MenuItemType type;
        int index;
    };

    static void fillStaticMenuItem(MenuItemType type, MENUITEMINFOW &info);
    static void getPlainMenuItemName(MenuBarHandle handle, UINT id, MutableString &name);
    static void setMenuItemState(Win32MenuItem *item, const MENUITEMINFOW *info);
    static void getMenuItemState(Win32MenuItem *item, MENUITEMINFOW *info);

    void createAllMenus() override;
    MenuBarHandle createMenuBar() override;
    MenuBarHandle createMenuBar(MenuItemType type) override;
    MenuItemHandle createMenuItem(MainMenuBarHandle menu) override;
    MenuItemHandle appendMenuItem(MenuBarHandle menu, MenuItemType type) override;
    void createSelectAccessoryMenuItem(MenuBarHandle menu, uint16_t handle, const char *name) override;
    void createSelectModelMenuItem(MenuBarHandle menu, uint16_t handle, const char *name) override;
    void createSelectBoneMenuItem(MenuBarHandle menu, const char *name) override;
    void createSelectMorphMenuItem(
        MenuBarHandle menu, nanoem_model_morph_category_t category, const char *name) override;
    void createPluginMenuItem(
        MenuBarHandle menu, MenuItemType type, uint16_t handle, const String &name, const StringList &items) override;
    void updateAllSelectDrawableItems(MenuBarHandle menu, uint16_t handle) override;
    void removeMenuItemById(MenuBarHandle menu, int index) noexcept override;

    void appendMenuSeparator(MenuBarHandle menu) override;
    void clearAllMenuItems(MenuBarHandle menu) override;
    void setParentMenu(MenuItemHandle parent, MenuBarHandle menu) override;
    void setMenuItemEnabled(MenuItemHandle item, bool value) override;
    void setMenuItemChecked(MenuItemHandle item, bool value) override;
    bool isMenuItemEnabled(MenuItemHandle item) const noexcept override;
    bool isMenuItemChecked(MenuItemHandle item) const noexcept override;

    void createWindowMenu(MainMenuBarHandle mainMenu);
    void createHelpMenu(MainMenuBarHandle mainMenu);
    void clearAllMenuItems() noexcept;
    void fillDynamicMenuItem(const Lambda &lambda, MENUITEMINFOW &info);

    const char *translateMenuItemWin32(MenuItemType type) const noexcept;
    const wchar_t *localizedMenuItemString(const char *text, const char *shortcut) const noexcept;

    using TranslatedMessageCache = tinystl::unordered_map<const char *, MutableWideString, TinySTLAllocator>;
    using Win32MenuList = tinystl::vector<Win32Menu *, TinySTLAllocator>;
    using Win32MenuIemList = tinystl::vector<Win32MenuItem *, TinySTLAllocator>;
    mutable TranslatedMessageCache m_localizedMessageCache;
    Win32MenuList m_menuInstances;
    Win32MenuIemList m_menuItemInstances;
    const ITranslator *m_translator;
    MainWindow *m_mainWindow;
    bx::HandleAlloc *m_allocator = nullptr;
    std::unordered_map<UINT, Lambda> m_lambdas;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_WIN32APPLICATIONMENUBUILDER_H_ */
