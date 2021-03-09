/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "Win32ApplicationMenuBuilder.h"

#include "MainWindow.h"

#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationClient.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"

namespace nanoem {
namespace win32 {

Win32ApplicationMenuBuilder::Win32ApplicationMenuBuilder(
    MainWindow *mainWindow, ThreadedApplicationClient *client, const ITranslator *translator, bool enableModelEditing)
    : ApplicationMenuBuilder(client, enableModelEditing)
    , m_translator(translator)
    , m_mainWindow(mainWindow)
{
    m_allocator = bx::createHandleAlloc(g_emapp_allocator, 0xffff);
    m_allocator->alloc();
}

Win32ApplicationMenuBuilder::~Win32ApplicationMenuBuilder()
{
    clearAllMenuItems();
    bx::destroyHandleAlloc(g_emapp_allocator, m_allocator);
}

void
Win32ApplicationMenuBuilder::dispatch(UINT menuItemID)
{
    auto it = m_lambdas.find(menuItemID);
    if (it != m_lambdas.end()) {
        it->second();
    }
    else {
        m_client->sendMenuActionMessage(menuItemID);
    }
}

void
Win32ApplicationMenuBuilder::fillStaticMenuItem(MenuItemType type, MENUITEMINFOW &info)
{
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STRING | MIIM_DATA | MIIM_STATE | MIIM_ID;
    info.fState = MFS_ENABLED;
    info.fType = MFT_STRING;
    info.wID = type;
}

void
Win32ApplicationMenuBuilder::getPlainMenuItemName(MenuBarHandle handle, UINT id, MutableString &name)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(handle);
    MENUITEMINFOW item = {};
    item.cbSize = sizeof(item);
    item.fState = MFT_STRING;
    item.fMask = MIIM_STRING;
    MutableWideString ws;
    HMENU hmenu = mu->parentMenu;
    GetMenuItemInfoW(hmenu, id, FALSE, &item);
    item.cch++;
    ws.resize(item.cch);
    item.dwTypeData = ws.data();
    GetMenuItemInfoW(hmenu, id, FALSE, &item);
    StringUtils::getMultiBytesString(ws.data(), name);
}

void
Win32ApplicationMenuBuilder::setMenuItemState(Win32MenuItem *item, const MENUITEMINFOW *info)
{
    if (item->type != kMenuItemTypeMaxEnum) {
        SetMenuItemInfoW(item->parentMenu, static_cast<UINT>(item->type), FALSE, info);
    }
    else {
        SetMenuItemInfoW(item->parentMenu, static_cast<UINT>(item->index), TRUE, info);
    }
}

void
Win32ApplicationMenuBuilder::getMenuItemState(Win32MenuItem *item, MENUITEMINFOW *info)
{
    if (item->type != kMenuItemTypeMaxEnum) {
        GetMenuItemInfoW(item->parentMenu, static_cast<UINT>(item->type), FALSE, info);
    }
    else {
        GetMenuItemInfoW(item->parentMenu, static_cast<UINT>(item->index), TRUE, info);
    }
}

void
Win32ApplicationMenuBuilder::createAllMenus()
{
    clearAllMenuItems();
    initialize();
    MainMenuBarHandle handle = reinterpret_cast<MainMenuBarHandle>(m_mainWindow->menuHandle());
    createFileMenu(handle);
    createEditMenu(handle);
    appendMenuSeparator(m_editMenu);
    appendMenuItem(m_editMenu, kMenuItemTypeEditPreference);
    createProjectMenu(handle);
    createCameraMenu(handle);
    createLightMenu(handle);
    createModelMenu(handle);
    createAccessoryMenu(handle);
    createWindowMenu(handle);
    createHelpMenu(handle);
    DrawMenuBar(m_mainWindow->windowHandle());
}

ApplicationMenuBuilder::MenuBarHandle
Win32ApplicationMenuBuilder::createMenuBar()
{
    auto item = new Win32Menu { CreatePopupMenu(), kMenuItemTypeMaxEnum };
    m_menuInstances.push_back(item);
    return reinterpret_cast<MenuBarHandle>(item);
}

ApplicationMenuBuilder::MenuBarHandle
Win32ApplicationMenuBuilder::createMenuBar(MenuItemType type)
{
    auto item = new Win32Menu { CreatePopupMenu(), type };
    m_menuInstances.push_back(item);
    return reinterpret_cast<MenuBarHandle>(item);
}

ApplicationMenuBuilder::MenuItemHandle
Win32ApplicationMenuBuilder::createMenuItem(MainMenuBarHandle menu)
{
    HMENU hmenu = reinterpret_cast<HMENU>(menu);
    auto item = new Win32MenuItem { hmenu, kMenuItemTypeMaxEnum, GetMenuItemCount(hmenu) };
    m_menuItemInstances.push_back(item);
    return reinterpret_cast<MenuItemHandle>(item);
}

ApplicationMenuBuilder::MenuItemHandle
Win32ApplicationMenuBuilder::appendMenuItem(MenuBarHandle menu, MenuItemType type)
{
    struct ShortcutMapping {
        MenuItemType type;
        const char *value;
    };
    static const ShortcutMapping kShortcutMapping[] = {
        { kMenuItemTypeFileNewProject, "Ctrl+N" },
        { kMenuItemTypeFileOpenProject, "Ctrl+O" },
        { kMenuItemTypeFileSaveProject, "Ctrl+S" },
        { kMenuItemTypeFileSaveAsProject, "Ctrl+Shift+S" },
        { kMenuItemTypeFileExportImage, "Ctrl+P" },
        { kMenuItemTypeFileExportVideo, "Ctrl+Shift+P" },
        { kMenuItemTypeFileExit, "Alt+F4" },
        { kMenuItemTypeEditUndo, "Ctrl+Z" },
        { kMenuItemTypeEditRedo, "Ctrl+Y" },
        { kMenuItemTypeEditCut, "Ctrl+X" },
        { kMenuItemTypeEditCopy, "Ctrl+C" },
        { kMenuItemTypeEditPaste, "Ctrl+V" },
        { kMenuItemTypeEditSelectAll, "Ctrl+A" },
        { kMenuItemTypeProjectPlay, "Ctrl+Space" },
        { kMenuItemTypeProjectStop, "Ctrl+." },
        { kMenuItemTypeCameraPresetTop, "Ctrl+5" },
        { kMenuItemTypeCameraPresetLeft, "Ctrl+4" },
        { kMenuItemTypeCameraPresetRight, "Ctrl+6" },
        { kMenuItemTypeCameraPresetFront, "Ctrl+2" },
        { kMenuItemTypeCameraPresetBack, "Ctrl+8" },
        { kMenuItemTypeWindowFullscreen, "Alt+Enter" },
    };
    const char *shortcut = "";
    for (const auto &it : kShortcutMapping) {
        if (it.type == type) {
            shortcut = it.value;
        }
    }
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info;
    fillStaticMenuItem(type, info);
    info.dwTypeData = const_cast<LPWSTR>(localizedMenuItemString(translateMenuItemWin32(type), shortcut));
    HMENU hmenu = mu->parentMenu;
    InsertMenuItemW(hmenu, static_cast<UINT>(type), FALSE, &info);
    auto item = new Win32MenuItem { hmenu, type, GetMenuItemCount(hmenu) };
    m_menuItems.insert(tinystl::make_pair(type, reinterpret_cast<MenuItemHandle>(item)));
    m_menuItemInstances.push_back(item);
    return reinterpret_cast<MenuItemHandle>(item);
}

void
Win32ApplicationMenuBuilder::createSelectAccessoryMenuItem(MenuBarHandle menu, uint16_t handle, const char *name)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info;
    fillDynamicMenuItem([this, handle]() { m_client->sendSetActiveAccessoryMessage(handle); }, info);
    MutableWideString ws;
    StringUtils::getWideCharString(name, ws);
    info.dwTypeData = ws.data();
    HMENU hmenu = mu->parentMenu;
    auto item = new Win32MenuItem { hmenu, kMenuItemTypeMaxEnum, GetMenuItemCount(hmenu) };
    InsertMenuItemW(hmenu, static_cast<UINT>(item->index), TRUE, &info);
    m_menuItemInstances.push_back(item);
}

void
Win32ApplicationMenuBuilder::createSelectModelMenuItem(MenuBarHandle menu, uint16_t handle, const char *name)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info;
    fillDynamicMenuItem([this, handle]() { m_client->sendSetActiveModelMessage(handle); }, info);
    MutableWideString ws;
    StringUtils::getWideCharString(name, ws);
    info.dwTypeData = ws.data();
    HMENU hmenu = mu->parentMenu;
    auto item = new Win32MenuItem { hmenu, kMenuItemTypeMaxEnum, GetMenuItemCount(hmenu) };
    InsertMenuItemW(hmenu, static_cast<UINT>(item->index), TRUE, &info);
    m_menuItemInstances.push_back(item);
}

void
Win32ApplicationMenuBuilder::createSelectBoneMenuItem(MenuBarHandle menu, const char *name)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    HMENU hmenu = mu->parentMenu;
    MENUITEMINFOW info;
    const std::string s(name);
    fillDynamicMenuItem([this, s]() { m_client->sendSetActiveModelBoneMessage(s.c_str()); }, info);
    MutableWideString ws;
    StringUtils::getWideCharString(name, ws);
    info.dwTypeData = ws.data();
    auto item = new Win32MenuItem { hmenu, kMenuItemTypeMaxEnum, GetMenuItemCount(hmenu) };
    InsertMenuItemW(hmenu, static_cast<UINT>(item->index), TRUE, &info);
    m_menuItemInstances.push_back(item);
}

void
Win32ApplicationMenuBuilder::createSelectMorphMenuItem(
    MenuBarHandle menu, nanoem_model_morph_category_t /* category */, const char *name)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    HMENU hmenu = mu->parentMenu;
    MENUITEMINFOW info;
    const std::string s(name);
    fillDynamicMenuItem([this, s]() { m_client->sendSetActiveModelMorphMessage(s.c_str(), true); }, info);
    MutableWideString ws;
    StringUtils::getWideCharString(name, ws);
    info.dwTypeData = ws.data();
    auto item = new Win32MenuItem { hmenu, kMenuItemTypeMaxEnum, GetMenuItemCount(hmenu) };
    InsertMenuItemW(hmenu, static_cast<UINT>(item->index), TRUE, &info);
    m_menuItemInstances.push_back(item);
}

void
Win32ApplicationMenuBuilder::createPluginMenuItem(
    MenuBarHandle menu, MenuItemType type, uint16_t handle, const String &name, const StringList &items)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    HMENU base = CreatePopupMenu();
    nanoem_rsize_t offset = 0;
    for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it, offset++) {
        MENUITEMINFOW info;
        const nanoem_rsize_t functionIndex = offset;
        if (type == kMenuItemTypeModelPluginExecute) {
            fillDynamicMenuItem(
                [this, handle, functionIndex]() {
                    m_client->sendExecuteModelPluginMessage(handle, Inline::saturateInt32U(functionIndex));
                },
                info);
        }
        else if (type == kMenuItemTypeMotionPluginExecute) {
            fillDynamicMenuItem(
                [this, handle, functionIndex]() {
                    m_client->sendExecuteMotionPluginMessage(handle, Inline::saturateInt32U(functionIndex));
                },
                info);
        }
        if (type == kMenuItemTypeModelPluginExecute || type == kMenuItemTypeMotionPluginExecute) {
            MutableWideString ws;
            StringUtils::getWideCharString(it->c_str(), ws);
            info.dwTypeData = ws.data();
            auto item = new Win32MenuItem { base, kMenuItemTypeMaxEnum, GetMenuItemCount(base) };
            InsertMenuItemW(base, static_cast<UINT>(item->index), TRUE, &info);
            m_menuItemInstances.push_back(item);
        }
    }
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    info.hSubMenu = base;
    info.fMask = MIIM_STRING | MIIM_DATA | MIIM_SUBMENU;
    info.fType = MFT_STRING;
    MutableWideString ws;
    StringUtils::getWideCharString(name.c_str(), ws);
    info.dwTypeData = ws.data();
    auto item = new Win32MenuItem { mu->parentMenu, kMenuItemTypeMaxEnum, GetMenuItemCount(mu->parentMenu) };
    InsertMenuItemW(mu->parentMenu, static_cast<UINT>(item->index), TRUE, &info);
    m_menuItemInstances.push_back(item);
}

void
Win32ApplicationMenuBuilder::updateAllSelectDrawableItems(MenuBarHandle menu, uint16_t handle)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    HMENU hmenu = mu->parentMenu;
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    for (int i = 0, numItems = GetMenuItemCount(hmenu); i < numItems; i++) {
        info.fMask = MIIM_ID | MIIM_DATA;
        GetMenuItemInfoW(hmenu, static_cast<UINT>(i), TRUE, &info);
        bool selected = info.dwItemData == handle;
        info.fState = selected ? MFS_CHECKED : MFS_UNCHECKED;
        info.fMask = MIIM_STATE;
        SetMenuItemInfoW(hmenu, static_cast<UINT>(i), TRUE, &info);
    }
}

void
Win32ApplicationMenuBuilder::removeMenuItemById(MenuBarHandle menu, int index)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    info.fMask = MIIM_ID;
    GetMenuItemInfoW(mu->parentMenu, static_cast<UINT>(index), TRUE, &info);
    if (info.wID >= kMenuItemTypeMaxEnum) {
        uint16_t id = info.wID - kMenuItemTypeMaxEnum;
        m_lambdas.erase(id);
        m_allocator->free(id);
    }
    DeleteMenu(mu->parentMenu, static_cast<UINT>(index), MF_BYPOSITION);
}

void
Win32ApplicationMenuBuilder::appendMenuSeparator(MenuBarHandle menu)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_TYPE;
    info.fType = MFT_SEPARATOR;
    HMENU hmenu = mu->parentMenu;
    InsertMenuItemW(hmenu, static_cast<UINT>(GetMenuItemCount(hmenu)), TRUE, &info);
}

void
Win32ApplicationMenuBuilder::clearAllMenuItems(MenuBarHandle menu)
{
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    HMENU hmenu = mu->parentMenu;
    for (int i = GetMenuItemCount(hmenu) - 1; i >= 0; i--) {
        DeleteMenu(hmenu, static_cast<UINT>(i), MF_BYPOSITION);
    }
}

void
Win32ApplicationMenuBuilder::setParentMenu(MenuItemHandle parent, MenuBarHandle menu)
{
    Win32MenuItem *mi = reinterpret_cast<Win32MenuItem *>(parent);
    Win32Menu *mu = reinterpret_cast<Win32Menu *>(menu);
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    info.hSubMenu = mu->parentMenu;
    info.fMask = MIIM_STRING | MIIM_DATA | MIIM_SUBMENU;
    info.fType = MFT_STRING;
    MenuItemType type = mu->type;
    /* remove duplication */
    if (type == kMenuItemTypeMaxEnum) {
        type = mi->type;
        DeleteMenu(mi->parentMenu, static_cast<UINT>(mi->index - 1), MF_BYPOSITION);
    }
    info.dwTypeData = const_cast<LPWSTR>(localizedMenuItemString(translateMenuItemWin32(type), nullptr));
    InsertMenuItemW(mi->parentMenu, static_cast<UINT>(mi->index), TRUE, &info);
}

void
Win32ApplicationMenuBuilder::setMenuItemEnabled(MenuItemHandle item, bool value)
{
    Win32MenuItem *mi = reinterpret_cast<Win32MenuItem *>(item);
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    info.fState = value ? MFS_ENABLED : MFS_DISABLED;
    info.fMask = MIIM_STATE;
    setMenuItemState(mi, &info);
}

void
Win32ApplicationMenuBuilder::setMenuItemChecked(MenuItemHandle item, bool value)
{
    Win32MenuItem *mi = reinterpret_cast<Win32MenuItem *>(item);
    MENUITEMINFOW info = {};
    info.cbSize = sizeof(info);
    info.fState = value ? MFS_CHECKED : MFS_UNCHECKED;
    info.fMask = MIIM_STATE;
    setMenuItemState(mi, &info);
}

bool
Win32ApplicationMenuBuilder::isMenuItemEnabled(MenuItemHandle item) const noexcept
{
    Win32MenuItem *mi = reinterpret_cast<Win32MenuItem *>(item);
    MENUITEMINFOW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STATE;
    getMenuItemState(mi, &info);
    return info.fState == MFS_ENABLED;
}

bool
Win32ApplicationMenuBuilder::isMenuItemChecked(MenuItemHandle item) const noexcept
{
    Win32MenuItem *mi = reinterpret_cast<Win32MenuItem *>(item);
    MENUITEMINFOW info;
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STATE;
    getMenuItemState(mi, &info);
    return info.fState == MFS_CHECKED;
}

void
Win32ApplicationMenuBuilder::createWindowMenu(MainMenuBarHandle mainMenu)
{
    MenuItemHandle windowMenu = createMenuItem(mainMenu);
    m_windowMenu = createMenuBar(kMenuItemTypeWindowTitle);
    setParentMenu(windowMenu, m_windowMenu);
    appendMenuItem(m_windowMenu, kMenuItemTypeWindowMaximize);
    appendMenuItem(m_windowMenu, kMenuItemTypeWindowMinimize);
    appendMenuItem(m_windowMenu, kMenuItemTypeWindowRestore);
    appendMenuSeparator(m_windowMenu);
    appendMenuItem(m_windowMenu, kMenuItemTypeWindowFullscreen);
}

void
Win32ApplicationMenuBuilder::createHelpMenu(MainMenuBarHandle mainMenu)
{
    MenuItemHandle helpMenu = createMenuItem(mainMenu);
    m_helpMenu = createMenuBar(kMenuItemTypeHelpTitle);
    setParentMenu(helpMenu, m_helpMenu);
    appendMenuItem(m_helpMenu, kMenuItemTypeHelpOnline);
    appendMenuSeparator(m_helpMenu);
    appendMenuItem(m_helpMenu, kMenuItemTypeHelpAbout);
}

void
Win32ApplicationMenuBuilder::clearAllMenuItems()
{
    for (auto it : m_menuItemInstances) {
        delete it;
    }
    m_menuItemInstances.clear();
    for (auto it : m_menuInstances) {
        delete it;
    }
    m_menuInstances.clear();
    HMENU menu = m_mainWindow->menuHandle();
    for (int i = GetMenuItemCount(menu); i >= 0; i--) {
        DeleteMenu(menu, static_cast<UINT>(i), MF_BYPOSITION);
    }
    m_lambdas.clear();
    m_localizedMessageCache.clear();
    m_allocator->reset();
    m_allocator->alloc();
}

void
Win32ApplicationMenuBuilder::fillDynamicMenuItem(const Lambda &lambda, MENUITEMINFOW &info)
{
    ZeroMemory(&info, sizeof(info));
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STRING | MIIM_DATA | MIIM_STATE | MIIM_ID | MIIM_DATA;
    info.fState = MFS_ENABLED;
    info.fType = MFT_STRING;
    info.wID = kMenuItemTypeMaxEnum + m_allocator->alloc();
    m_lambdas.insert(std::make_pair(info.wID, lambda));
}

const char *
Win32ApplicationMenuBuilder::translateMenuItemWin32(MenuItemType type) const
{
    char name[128];
    const char *id = menuItemString(type);
    bx::snprintf(name, sizeof(name), "%s.win32", id);
    return m_translator->translate(m_translator->isTranslatable(name) ? name : id);
}

const wchar_t *
Win32ApplicationMenuBuilder::localizedMenuItemString(const char *text, const char *shortcut) const
{
    auto it = m_localizedMessageCache.find(text);
    if (it == m_localizedMessageCache.end()) {
        MutableWideString translatedString;
        StringUtils::getWideCharString(text, translatedString);
        if (shortcut) {
            MutableWideString shortcutString;
            StringUtils::getWideCharString(shortcut, shortcutString);
            translatedString.pop_back();
            translatedString.push_back(L'\t');
            for (auto it = shortcutString.begin(), end = shortcutString.end(); it != end; ++it) {
                translatedString.push_back(*it);
            }
            translatedString.push_back(L'\0');
        }
        it = m_localizedMessageCache.insert(tinystl::make_pair(text, translatedString)).first;
    }
    return it->second.data();
}

} /* namespace win32 */
} /* namespace nanoem */
