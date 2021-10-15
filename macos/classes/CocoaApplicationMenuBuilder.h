/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef NANOEM_EMAPP_MACOS_MACOS_APPLICATIONMENUBUILDER_H_
#define NANOEM_EMAPP_MACOS_MACOS_APPLICATIONMENUBUILDER_H_

#import "MenuItemCollection.h"
#import <AppKit/AppKit.h>

#include "emapp/ApplicationMenuBuilder.h"
#include "emapp/IFileManager.h"
#include "emapp/ThreadedApplicationClient.h"

namespace nanoem {

class ITranslator;

namespace macos {

class MainWindow;
class Preference;

class CocoaApplicationMenuBuilder NANOEM_DECL_SEALED : public ApplicationMenuBuilder {
public:
    static NSOpenPanel *createOpenPanel(NSArray *allowedFileTypes);
    static NSOpenPanel *createOpenPanel(const StringList &extensions);
    static NSSavePanel *createSavePanel(NSArray *allowedFileTypes);
    static NSSavePanel *createSavePanel(const StringList &extensions);
    static NSArray *allowedFileTypesFromStringList(const StringList &extensions);
    static void aggregateAllPlugins(URIList &plugins);
    static void getAllowedProjectExtensions(StringList &extensions);
    static void clearModalPanel(NSPanel *panel, NSWindow *window);

    CocoaApplicationMenuBuilder(MainWindow *parent, ThreadedApplicationClient *client, const ITranslator *translator,
        const Preference *preference);
    ~CocoaApplicationMenuBuilder() noexcept;

    void openProject();
    void saveProject();
    void registerDocumentEditEventListener();

    MenuItemHandle appendMenuItem(MenuItemCollection *menu, MenuItemType type, NSString *keyEquivalent = @"");
    MenuItemHandle appendMenuItem(MenuItemCollection *menu, MenuItemType type, MenuItemActionItemCallback callback,
        NSString *keyEquivalent = @"");
    NSString *translatedString(MenuItemType type) const;

private:
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
    void clearAllMenuItems(MenuBarHandle menu) noexcept override;
    void setParentMenu(MenuItemHandle parent, MenuBarHandle menu) override;
    void setMenuItemEnabled(MenuItemHandle item, bool value) override;
    void setMenuItemChecked(MenuItemHandle item, bool value) override;
    bool isMenuItemEnabled(MenuItemHandle item) const noexcept override;
    bool isMenuItemChecked(MenuItemHandle item) const noexcept override;

    void createApplicationMenu(NSMenu *bar);
    void createFileMenu(NSMenu *bar);
    void createFileImportMenu(MenuItemCollection *fileMenu);
    void createFileExportMenu(MenuItemCollection *fileMenu);
    void createWindowMenu(NSMenu *bar, NSApplication *app);
    void createHelpMenu(NSMenu *bar, NSApplication *app);

    void newProject();
    void openProject(NSOpenPanel *panel);
    void loadFile(NSOpenPanel *panel, IFileManager::DialogType type);
    void saveFile(NSSavePanel *panel, IFileManager::DialogType type);
    void saveFile(const URI &fileURI, IFileManager::DialogType type);
    void exportImage();
    void exportVideo();
    NSString *translatedString(const char *text) const;
    URIList cachedAggregateAllPlugins();

    const ITranslator *m_translator;
    const Preference *m_preference;
    NSMutableArray *m_instances = [[NSMutableArray alloc] init];
    NSMutableArray *m_modelPluginMenus = [[NSMutableArray alloc] init];
    NSMutableArray *m_motionPluginMenus = [[NSMutableArray alloc] init];
    MainWindow *m_parent = nil;
    MenuItemCollection *m_appMenu = nil;
    MenuItemCollection *m_fileMenu = nil;
    MenuItemCollection *m_importMenu = nil;
    MenuItemCollection *m_exportMenu = nil;
    MenuItemCollection *m_windowMenu = nil;
    MenuItemCollection *m_helpMenu = nil;
    URIList m_cachedPluginURIs;
};

} /* namespace macos */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MACOS_MACOS_APPLICATIONMENUBUILDER_H_ */
