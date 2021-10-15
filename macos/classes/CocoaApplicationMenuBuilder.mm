/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import "CocoaApplicationMenuBuilder.h"

#import "CocoaThreadedApplicationService.h"
#import "MainWindow.h"
#import "Preference.h"

#include "emapp/Accessory.h"
#include "emapp/ITranslator.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/ThreadedApplicationClient.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace macos {

NSOpenPanel *
CocoaApplicationMenuBuilder::createOpenPanel(NSArray *allowedFileTypes)
{
    NSOpenPanel *panel = [NSOpenPanel openPanel];
    panel.allowedFileTypes = allowedFileTypes;
    return panel;
}

NSOpenPanel *
CocoaApplicationMenuBuilder::createOpenPanel(const StringList &extensions)
{
    return createOpenPanel(allowedFileTypesFromStringList(extensions));
}

NSSavePanel *
CocoaApplicationMenuBuilder::createSavePanel(NSArray *allowedFileTypes)
{
    NSSavePanel *panel = [NSSavePanel savePanel];
    panel.allowedFileTypes = allowedFileTypes;
    panel.showsTagField = YES;
    panel.tagNames = @[ @"nanoem" ];
    return panel;
}

NSSavePanel *
CocoaApplicationMenuBuilder::createSavePanel(const StringList &extensions)
{
    return createSavePanel(allowedFileTypesFromStringList(extensions));
}

NSArray *
CocoaApplicationMenuBuilder::allowedFileTypesFromStringList(const StringList &extensions)
{
    NSMutableArray *allowedFileTypes = nil;
    if (!extensions.empty()) {
        allowedFileTypes = [NSMutableArray arrayWithCapacity:extensions.size()];
        for (auto it : extensions) {
            NSString *item = [[NSString alloc] initWithUTF8String:it.c_str()];
            [allowedFileTypes addObject:item];
        }
    }
    return allowedFileTypes;
}

void
CocoaApplicationMenuBuilder::aggregateAllPlugins(URIList &plugins)
{
    NSURL *bundlePluginsURL = [NSBundle mainBundle].builtInPlugInsURL;
    NSDirectoryEnumerator *enumerator =
        [[NSFileManager defaultManager] enumeratorAtURL:bundlePluginsURL
                             includingPropertiesForKeys:nil
                                                options:NSDirectoryEnumerationSkipsHiddenFiles
                                           errorHandler:nil];
    for (NSURL *fileURL in enumerator) {
        if ([fileURL.pathExtension isEqualToString:@"dylib"]) {
            plugins.push_back(CocoaThreadedApplicationService::canonicalFileURI(fileURL));
        }
    }
}

void
CocoaApplicationMenuBuilder::getAllowedProjectExtensions(StringList &extensions)
{
    extensions = Project::loadableExtensions();
    extensions.push_back("nanoem");
}

void
CocoaApplicationMenuBuilder::clearModalPanel(NSPanel *panel, NSWindow *window)
{
    [panel orderOut:nil];
    [window endSheet:panel];
    [window makeKeyWindow];
}

CocoaApplicationMenuBuilder::CocoaApplicationMenuBuilder(
    MainWindow *parent, ThreadedApplicationClient *client, const ITranslator *translator, const Preference *preference)
    : ApplicationMenuBuilder(client, preference->applicationPreference()->isModelEditingEnabled())
    , m_translator(translator)
    , m_preference(preference)
    , m_parent(parent)
{
}

CocoaApplicationMenuBuilder::~CocoaApplicationMenuBuilder() noexcept
{
}

void
CocoaApplicationMenuBuilder::openProject()
{
    StringList extensions;
    getAllowedProjectExtensions(extensions);
    NSOpenPanel *panel = createOpenPanel(extensions);
    NSWindow *window = m_parent->nativeWindow();
    [panel beginSheetModalForWindow:window
                  completionHandler:^(NSInteger result) {
                      clearModalPanel(panel, window);
                      if (result == NSModalResponseOK) {
                          m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
                          loadFile(panel, IFileManager::kDialogTypeOpenProject);
                      }
                      else {
                          m_client->clearAllCompleteLoadingFileOnceEventListeners();
                      }
                  }];
}

void
CocoaApplicationMenuBuilder::saveProject()
{
    m_client->sendGetProjectFileURIRequestMessage(
        [](void *userData, const URI &fileURI) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            StringList extensions;
            getAllowedProjectExtensions(extensions);
            if (!fileURI.isEmpty() &&
                FileUtils::isLoadableExtension(fileURI.pathExtension(), ListUtils::toSetFromList(extensions))) {
                self->saveFile(fileURI, IFileManager::kDialogTypeSaveProjectFile);
                self->registerDocumentEditEventListener();
            }
            else {
                NSSavePanel *panel = createSavePanel(extensions);
                NSWindow *window = self->m_parent->nativeWindow();
                [panel beginSheetModalForWindow:window
                              completionHandler:^(NSInteger result) {
                                  clearModalPanel(panel, window);
                                  if (result == NSModalResponseOK) {
                                      self->m_parent->setTitle(panel.URL);
                                      self->saveFile(panel, IFileManager::kDialogTypeSaveProjectFile);
                                  }
                                  else {
                                      self->m_client->clearAllCompleteSavingFileOnceEventListeners();
                                  }
                              }];
            }
        },
        this);
}

void
CocoaApplicationMenuBuilder::registerDocumentEditEventListener()
{
    m_parent->nativeWindow().documentEdited = NO;
    m_client->addUndoChangeEventListener(
        [](void *userData) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            self->m_parent->nativeWindow().documentEdited = YES;
        },
        this, true);
}

ApplicationMenuBuilder::MenuItemHandle
CocoaApplicationMenuBuilder::appendMenuItem(MenuItemCollection *menu, MenuItemType type, NSString *keyEquivalent)
{
    return appendMenuItem(
        menu, type,
        ^() {
            m_client->sendMenuActionMessage(type);
        },
        keyEquivalent);
}

NSString *
CocoaApplicationMenuBuilder::translatedString(MenuItemType type) const
{
    char buffer[Inline::kLongNameStackBufferSize];
    const char *translated = stripMnemonic(buffer, sizeof(buffer), m_translator->translate(menuItemString(type)));
    NSString *title = [[NSString alloc] initWithUTF8String:translated];
    return title;
}

ApplicationMenuBuilder::MenuItemHandle
CocoaApplicationMenuBuilder::appendMenuItem(
    MenuItemCollection *menu, MenuItemType type, MenuItemActionItemCallback callback, NSString *keyEquivalent)
{
    NSMenuItem *item = [menu addItemWithTitle:translatedString(type) action:callback keyEquivalent:keyEquivalent];
    item.tag = type;
    MenuItemHandle handle = (__bridge MenuItemHandle) item;
    [m_instances addObject:item];
    m_menuItems.insert(tinystl::make_pair(type, handle));
    return handle;
}

void
CocoaApplicationMenuBuilder::createAllMenus()
{
    [m_instances removeAllObjects];
    NSApplication *app = [NSApplication sharedApplication];
    NSMenu *menu = [[NSMenu alloc] init];
    MainMenuBarHandle handle = (__bridge MainMenuBarHandle) menu;
    menu.autoenablesItems = NO;
    initialize();
    createApplicationMenu(menu);
    createFileMenu(menu);
    createEditMenu(handle);
    createProjectMenu(handle);
    createCameraMenu(handle);
    createLightMenu(handle);
    createModelMenu(handle);
    createAccessoryMenu(handle);
    createWindowMenu(menu, app);
    createHelpMenu(menu, app);
    setMenuItemEnable(kMenuItemTypeFileExportModel, false);
    setMenuItemEnable(kMenuItemTypeFileExportModelMotion, false);
    setMenuItemEnable(kMenuItemTypeFileExportModelPose, false);
    app.mainMenu = menu;
}

ApplicationMenuBuilder::MenuBarHandle
CocoaApplicationMenuBuilder::createMenuBar()
{
    MenuItemCollection *collection = [[MenuItemCollection alloc] initWithTitle:@""];
    [m_instances addObject:collection];
    return (__bridge MenuBarHandle) collection;
}

ApplicationMenuBuilder::MenuBarHandle
CocoaApplicationMenuBuilder::createMenuBar(MenuItemType type)
{
    MenuItemCollection *collection = [[MenuItemCollection alloc] initWithTitle:translatedString(type)];
    [m_instances addObject:collection];
    return (__bridge MenuBarHandle) collection;
}

ApplicationMenuBuilder::MenuItemHandle
CocoaApplicationMenuBuilder::createMenuItem(MainMenuBarHandle menu)
{
    NSMenuItem *item = [(__bridge NSMenu *) menu addItemWithTitle:@"" action:nil keyEquivalent:@""];
    [m_instances addObject:item];
    return (__bridge MenuItemHandle) item;
}

ApplicationMenuBuilder::MenuItemHandle
CocoaApplicationMenuBuilder::appendMenuItem(MenuBarHandle menu, MenuItemType type)
{
    struct ShortcutMapping {
        MenuItemType type;
        NSString *value;
    };
    static const ShortcutMapping kShortcutMapping[] = { { kMenuItemTypeEditUndo, @"z" },
        { kMenuItemTypeEditRedo, @"Z" }, { kMenuItemTypeEditCopy, @"c" }, { kMenuItemTypeEditCut, @"x" },
        { kMenuItemTypeEditPaste, @"v" }, { kMenuItemTypeCameraPresetTop, @"5" },
        { kMenuItemTypeCameraPresetLeft, @"4" }, { kMenuItemTypeCameraPresetRight, @"6" },
        { kMenuItemTypeCameraPresetBottom, @"1" }, { kMenuItemTypeCameraPresetFront, @"2" },
        { kMenuItemTypeCameraPresetBack, @"8" }, { kMenuItemTypeEditSelectAll, @"a" },
        { kMenuItemTypeProjectPlay, @" " }, { kMenuItemTypeProjectStop, @"." } };
    NSString *keyEquivalent = @"";
    for (const auto &it : kShortcutMapping) {
        if (it.type == type) {
            keyEquivalent = it.value;
            break;
        }
    }
    NSMenuItem *item = [(__bridge MenuItemCollection *) menu addItemWithTitle:translatedString(type)
                                                                       action:^() {
                                                                           m_client->sendMenuActionMessage(type);
                                                                       }
                                                                keyEquivalent:keyEquivalent];
    MenuItemHandle handle = (__bridge MenuItemHandle) item;
    m_menuItems.insert(tinystl::make_pair(type, handle));
    [m_instances addObject:item];
    return handle;
}

void
CocoaApplicationMenuBuilder::createSelectAccessoryMenuItem(MenuBarHandle menu, uint16_t handle, const char *name)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSString *nameString = [[NSString alloc] initWithUTF8String:name];
    NSMenuItem *menuItem = [collection addItemWithTitle:nameString
                                                 action:^() {
                                                     m_client->sendSetActiveAccessoryMessage(handle);
                                                 }];
    menuItem.tag = handle;
}

void
CocoaApplicationMenuBuilder::createSelectModelMenuItem(MenuBarHandle menu, uint16_t handle, const char *name)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSString *nameString = [[NSString alloc] initWithUTF8String:name];
    NSMenuItem *menuItem = [collection addItemWithTitle:nameString
                                                 action:^() {
                                                     m_client->sendSetActiveModelMessage(handle);
                                                 }];
    menuItem.tag = handle;
}

void
CocoaApplicationMenuBuilder::createSelectBoneMenuItem(MenuBarHandle menu, const char *name)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSString *nameString = [[NSString alloc] initWithUTF8String:name];
    [collection addItemWithTitle:nameString
                          action:^() {
                              m_client->sendSetActiveModelBoneMessage(name);
                          }
                   keyEquivalent:@""];
}

void
CocoaApplicationMenuBuilder::createSelectMorphMenuItem(
    MenuBarHandle menu, nanoem_model_morph_category_t /* category */, const char *name)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSString *nameString = [[NSString alloc] initWithUTF8String:name];
    [collection addItemWithTitle:nameString
                          action:^() {
                              m_client->sendSetActiveModelMorphMessage(name, true);
                          }
                   keyEquivalent:@""];
}

void
CocoaApplicationMenuBuilder::createPluginMenuItem(
    MenuBarHandle menu, MenuItemType type, uint16_t handle, const String &name, const StringList &items)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSString *pluginName = [[NSString alloc] initWithUTF8String:name.c_str()];
    NSMenuItem *item = [collection addItemWithTitle:pluginName];
    MenuItemCollection *pluginMenu = [[MenuItemCollection alloc] initWithTitle:@""];
    nanoem_rsize_t offset = 0;
    [pluginMenu setParentMenu:item];
    switch (type) {
    case kMenuItemTypeModelPluginExecute: {
        for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it, ++offset) {
            NSString *functionName = [[NSString alloc] initWithUTF8String:it->c_str()];
            __block const nanoem_rsize_t functionIndex = offset;
            [pluginMenu addItemWithTitle:functionName
                                  action:^() {
                                      m_client->sendExecuteModelPluginMessage(handle, functionIndex);
                                  }
                           keyEquivalent:@""];
        }
        [m_modelPluginMenus addObject:pluginMenu];
        break;
    }
    case kMenuItemTypeMotionPluginExecute: {
        for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it, ++offset) {
            NSString *functionName = [[NSString alloc] initWithUTF8String:it->c_str()];
            __block const nanoem_rsize_t functionIndex = offset;
            [pluginMenu addItemWithTitle:functionName
                                  action:^() {
                                      m_client->sendExecuteMotionPluginMessage(handle, functionIndex);
                                  }];
        }
        [m_motionPluginMenus addObject:pluginMenu];
        break;
    }
    default:
        break;
    }
}

void
CocoaApplicationMenuBuilder::updateAllSelectDrawableItems(MenuBarHandle menu, uint16_t handle)
{
    for (NSMenuItem *menuItem in ((__bridge MenuItemCollection *) menu).menu.itemArray) {
        menuItem.state = menuItem.tag == handle ? NSOnState : NSOffState;
    }
}

void
CocoaApplicationMenuBuilder::removeMenuItemById(MenuBarHandle menu, int index) noexcept
{
    auto collection = (__bridge MenuItemCollection *) menu;
    NSMenu *parent = collection.menu;
    NSArray *menuItems = parent.itemArray;
    for (NSUInteger i = 0, numMenuItems = menuItems.count; i < numMenuItems; i++) {
        NSMenuItem *menuItem = menuItems[i];
        if (menuItem.tag == index) {
            [parent removeItemAtIndex:i];
            break;
        }
    }
}

void
CocoaApplicationMenuBuilder::appendMenuSeparator(MenuBarHandle menu)
{
    auto collection = (__bridge MenuItemCollection *) menu;
    [collection addSeparator];
}

void
CocoaApplicationMenuBuilder::clearAllMenuItems(MenuBarHandle menu) noexcept
{
    auto collection = (__bridge MenuItemCollection *) menu;
    [collection removeAllItems];
    if (menu == m_editModelPluginMenu) {
        [m_modelPluginMenus removeAllObjects];
    }
    else if (menu == m_editMotionPluginMenu) {
        [m_motionPluginMenus removeAllObjects];
    }
}

void
CocoaApplicationMenuBuilder::setParentMenu(MenuItemHandle parent, MenuBarHandle item)
{
    auto collection = (__bridge MenuItemCollection *) item;
    [collection setParentMenu:(__bridge NSMenuItem *) parent];
}

void
CocoaApplicationMenuBuilder::setMenuItemEnabled(MenuItemHandle item, bool value)
{
    ((__bridge NSMenuItem *) item).enabled = value ? YES : NO;
}

void
CocoaApplicationMenuBuilder::setMenuItemChecked(MenuItemHandle item, bool value)
{
    ((__bridge NSMenuItem *) item).state = value ? NSOnState : NSOffState;
}

bool
CocoaApplicationMenuBuilder::isMenuItemEnabled(MenuItemHandle item) const noexcept
{
    return ((__bridge NSMenuItem *) item).enabled == YES;
}

bool
CocoaApplicationMenuBuilder::isMenuItemChecked(MenuItemHandle item) const noexcept
{
    return ((__bridge NSMenuItem *) item).state == NSOnState;
}

void
CocoaApplicationMenuBuilder::createApplicationMenu(NSMenu *bar)
{
    NSMenuItem *appMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
    m_appMenu = [[MenuItemCollection alloc] initWithTitle:@""];
    [m_appMenu setParentMenu:appMenuItem];
    [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.about")
                         action:^() {
                             NSApplication *app = [NSApplication sharedApplication];
                             [app orderFrontStandardAboutPanel:app];
                         }];
    [m_appMenu addSeparator];
    NSMenuItem *preferenceMenuItem =
        [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.preferences")
                             action:^() {
                                 m_client->sendMenuActionMessage(kMenuItemTypeEditPreference);
                             }
                      keyEquivalent:@","];
    preferenceMenuItem.tag = kMenuItemTypeEditPreference;
    m_menuItems.insert(tinystl::make_pair(kMenuItemTypeEditPreference, (__bridge MenuItemHandle) preferenceMenuItem));
    [m_appMenu addSeparator];
    [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.hide")
                         action:^() {
                             NSApplication *app = [NSApplication sharedApplication];
                             [app hide:app];
                         }
                  keyEquivalent:@"h"];
    [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.hide-all")
                         action:^() {
                             NSApplication *app = [NSApplication sharedApplication];
                             [app hideOtherApplications:app];
                         }
                  keyEquivalent:@"H"];
    [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.show-all")
                         action:^() {
                             NSApplication *app = [NSApplication sharedApplication];
                             [app unhideAllApplications:app];
                         }];
    [m_appMenu addSeparator];
    [m_appMenu addItemWithTitle:translatedString("nanoem.menu.macos.quit")
                         action:^() {
                             NSApplication *app = [NSApplication sharedApplication];
                             [app terminate:app];
                         }
                  keyEquivalent:@"q"];
}

void
CocoaApplicationMenuBuilder::createFileMenu(NSMenu *bar)
{
    NSMenuItem *fileMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
    m_fileMenu = [[MenuItemCollection alloc] initWithTitle:translatedString(kMenuItemTypeFileTitle)];
    [m_fileMenu setParentMenu:fileMenuItem];
    appendMenuItem(
        m_fileMenu, kMenuItemTypeFileNewProject,
        ^() {
            m_client->sendIsProjectDirtyRequestMessage(
                [](void *userData, bool dirty) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    BaseApplicationClient *client = self->m_client;
                    if (dirty) {
                        client->clearAllProjectAfterConfirmOnceEventListeners();
                        client->addSaveProjectAfterConfirmEventListener(
                            [](void *userData) {
                                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                self->m_client->addCompleteSavingFileEventListener(
                                    [](void *userData, const URI & /* fileURI */, uint32_t /* type */,
                                        uint64_t /* ticks */) {
                                        auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                        self->newProject();
                                        self->registerDocumentEditEventListener();
                                    },
                                    self, true);
                                self->saveProject();
                            },
                            self, true);
                        client->addDiscardProjectAfterConfirmEventListener(
                            [](void *userData) {
                                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                self->newProject();
                                self->registerDocumentEditEventListener();
                            },
                            self, true);
                        client->sendConfirmBeforeNewProjectMessage();
                    }
                    else {
                        self->newProject();
                    }
                },
                this);
        },
        @"n");
    if (m_preference->applicationPreference()->isModelEditingEnabled()) {
        appendMenuItem(m_fileMenu, kMenuItemTypeFileNewModel);
    }
    [m_fileMenu addSeparator];
    appendMenuItem(
        m_fileMenu, kMenuItemTypeFileOpenProject,
        ^() {
            m_client->sendIsProjectDirtyRequestMessage(
                [](void *userData, bool dirty) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    BaseApplicationClient *client = self->m_client;
                    if (dirty) {
                        client->clearAllProjectAfterConfirmOnceEventListeners();
                        client->addSaveProjectAfterConfirmEventListener(
                            [](void *userData) {
                                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                self->m_client->addCompleteSavingFileEventListener(
                                    [](void *userData, const URI &fileURI, uint32_t type, uint64_t /* ticks */) {
                                        BX_UNUSED_2(type, fileURI);
                                        auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                        self->openProject();
                                    },
                                    self, true);
                                self->saveProject();
                                self->registerDocumentEditEventListener();
                            },
                            self, true);
                        client->addDiscardProjectAfterConfirmEventListener(
                            [](void *userData) {
                                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                                self->openProject();
                                self->registerDocumentEditEventListener();
                            },
                            self, true);
                        client->sendConfirmBeforeOpenProjectMessage();
                    }
                    else {
                        self->openProject();
                    }
                },
                this);
        },
        @"o");
    createFileImportMenu(m_fileMenu);
    [m_fileMenu addSeparator];
    appendMenuItem(
        m_fileMenu, kMenuItemTypeFileSaveProject,
        ^() {
            saveProject();
        },
        @"s");
    appendMenuItem(
        m_fileMenu, kMenuItemTypeFileSaveAsProject,
        ^() {
            StringList extensions;
            getAllowedProjectExtensions(extensions);
            NSWindow *window = m_parent->nativeWindow();
            NSSavePanel *panel = createSavePanel(extensions);
            [panel beginSheetModalForWindow:window
                          completionHandler:^(NSInteger result) {
                              clearModalPanel(panel, window);
                              if (result == NSModalResponseOK) {
                                  m_parent->setTitle(panel.URL);
                                  saveFile(panel, IFileManager::kDialogTypeSaveProjectFile);
                              }
                          }];
        },
        @"S");
    [m_fileMenu addSeparator];
    createFileExportMenu(m_fileMenu);
}

void
CocoaApplicationMenuBuilder::createFileImportMenu(MenuItemCollection *fileMenu)
{
    NSWindow *window = m_parent->nativeWindow();
    NSMenuItem *openMenuItem = [fileMenu addItemWithTitle:translatedString(kMenuItemTypeFileImportTitle)];
    m_importMenu = [[MenuItemCollection alloc] initWithTitle:@""];
    [m_importMenu setParentMenu:openMenuItem];
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportModel, ^() {
        StringList extensions(Model::loadableExtensions());
        extensions.push_back("txt");
        NSOpenPanel *panel = createOpenPanel(extensions);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenModelFile);
                          }
                      }];
    });
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportAccessory, ^() {
        StringList extensions(Accessory::loadableExtensions());
        extensions.push_back("txt");
        NSOpenPanel *panel = createOpenPanel(extensions);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenModelFile);
                          }
                      }];
    });
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportModelPose, ^() {
        NSOpenPanel *panel = createOpenPanel(@[ @"vpd" ]);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenModelMotionFile);
                          }
                      }];
    });
    [m_importMenu addSeparator];
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportModelMotion, ^() {
        StringList extensions(Motion::loadableExtensions());
        extensions.push_back("txt");
        NSOpenPanel *panel = createOpenPanel(extensions);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenModelMotionFile);
                          }
                      }];
    });
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportCameraMotion, ^() {
        StringList extensions(Motion::loadableExtensions());
        extensions.push_back("txt");
        NSOpenPanel *panel = createOpenPanel(extensions);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenCameraMotionFile);
                          }
                      }];
    });
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportLightMotion, ^() {
        StringList extensions(Motion::loadableExtensions());
        extensions.push_back("txt");
        NSOpenPanel *panel = createOpenPanel(extensions);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              loadFile(panel, IFileManager::kDialogTypeOpenLightMotionFile);
                          }
                      }];
    });
    [m_importMenu addSeparator];
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportAudioSource, ^() {
        m_client->addAvailableAllImportingAudioExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                NSWindow *window = self->m_parent->nativeWindow();
                StringList newExtensions(extensions);
                newExtensions.push_back("wav");
                NSOpenPanel *panel = createOpenPanel(newExtensions);
                [panel beginSheetModalForWindow:window
                              completionHandler:^(NSInteger result) {
                                  clearModalPanel(panel, window);
                                  if (result == NSModalResponseOK) {
                                      self->loadFile(panel, IFileManager::kDialogTypeOpenAudioFile);
                                  }
                              }];
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
    });
    appendMenuItem(m_importMenu, kMenuItemTypeFileImportBackgroundVideo, ^() {
        m_client->addAvailableAllImportingVideoExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                NSWindow *window = self->m_parent->nativeWindow();
                StringList newExtensions(extensions);
                newExtensions.push_back("mp4");
                newExtensions.push_back("mov");
                NSOpenPanel *panel = createOpenPanel(newExtensions);
                [panel beginSheetModalForWindow:window
                              completionHandler:^(NSInteger result) {
                                  clearModalPanel(panel, window);
                                  if (result == NSModalResponseOK) {
                                      self->loadFile(panel, IFileManager::kDialogTypeOpenVideoFile);
                                  }
                              }];
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(cachedAggregateAllPlugins());
    });
}

void
CocoaApplicationMenuBuilder::createFileExportMenu(MenuItemCollection *fileMenu)
{
    NSWindow *window = m_parent->nativeWindow();
    NSMenuItem *exportMenuItem = [fileMenu addItemWithTitle:translatedString(kMenuItemTypeFileExportTitle)];
    m_exportMenu = [[MenuItemCollection alloc] initWithTitle:@""];
    [m_exportMenu setParentMenu:exportMenuItem];
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModelPose, ^() {
        NSSavePanel *panel = createSavePanel(@[ @"vpd" ]);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              saveFile(panel, IFileManager::kDialogTypeSaveModelMotionFile);
                          }
                      }];
    });
    [m_exportMenu addSeparator];
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModelMotion, ^() {
        NSSavePanel *panel = createSavePanel(@[ @"nmd", @"vmd" ]);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              saveFile(panel, IFileManager::kDialogTypeSaveModelMotionFile);
                          }
                      }];
    });
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportCameraMotion, ^() {
        NSSavePanel *panel = createSavePanel(@[ @"nmd", @"vmd" ]);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              saveFile(panel, IFileManager::kDialogTypeSaveCameraMotionFile);
                          }
                      }];
    });
    appendMenuItem(m_exportMenu, kMenuItemTypeFileExportLightMotion, ^() {
        NSSavePanel *panel = createSavePanel(@[ @"nmd", @"vmd" ]);
        [panel beginSheetModalForWindow:window
                      completionHandler:^(NSInteger result) {
                          clearModalPanel(panel, window);
                          if (result == NSModalResponseOK) {
                              saveFile(panel, IFileManager::kDialogTypeSaveLightMotionFile);
                          }
                      }];
    });
    if (m_preference->applicationPreference()->isModelEditingEnabled()) {
        [m_exportMenu addSeparator];
        appendMenuItem(m_exportMenu, kMenuItemTypeFileExportModel, ^() {
            NSSavePanel *panel = createSavePanel(@[ @"pmx" ]);
            [panel beginSheetModalForWindow:window
                          completionHandler:^(NSInteger result) {
                              clearModalPanel(panel, window);
                              if (result == NSModalResponseOK) {
                                  saveFile(panel, IFileManager::kDialogTypeSaveModelFile);
                              }
                          }];
        });
    }
    [m_exportMenu addSeparator];
    appendMenuItem(
        m_exportMenu, kMenuItemTypeFileExportImage,
        ^() {
            m_client->addSaveProjectAfterConfirmEventListener(
                [](void *userData) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    self->m_client->addCompleteSavingFileEventListener(
                        [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                            self->exportImage();
                        },
                        self, true);
                    self->saveProject();
                },
                this, true);
            m_client->addDiscardProjectAfterConfirmEventListener(
                [](void *userData) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    self->exportImage();
                },
                this, true);
            m_client->sendConfirmBeforeExportingImageMessage();
        },
        @"p");
    appendMenuItem(
        m_exportMenu, kMenuItemTypeFileExportVideo,
        ^() {
            m_client->addSaveProjectAfterConfirmEventListener(
                [](void *userData) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    self->m_client->addCompleteSavingFileEventListener(
                        [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
                            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                            self->exportVideo();
                        },
                        self, true);
                    self->saveProject();
                },
                this, true);
            m_client->addDiscardProjectAfterConfirmEventListener(
                [](void *userData) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    self->exportVideo();
                },
                this, true);
            m_client->sendConfirmBeforeExportingVideoMessage();
        },
        @"P");
}

void
CocoaApplicationMenuBuilder::createWindowMenu(NSMenu *bar, NSApplication *app)
{
    NSWindow *window = m_parent->nativeWindow();
    NSMenuItem *windowMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
    m_windowMenu = [[MenuItemCollection alloc] initWithTitle:translatedString(kMenuItemTypeWindowTitle)];
    [m_windowMenu setParentMenu:windowMenuItem];
    [m_windowMenu addItemWithTitle:translatedString("nanoem.menu.macos.window.minimize")
                            action:^() {
                                [window performMiniaturize:window];
                            }
                     keyEquivalent:@"m"];
    [m_windowMenu addItemWithTitle:translatedString("nanoem.menu.macos.window.zoom")
                            action:^() {
                                [window performZoom:window];
                            }];
    [m_windowMenu addSeparator];
    [m_windowMenu addItemWithTitle:translatedString("nanoem.menu.macos.window.bring-all-to-front")
                            action:^() {
                                NSApplication *app = [NSApplication sharedApplication];
                                [app arrangeInFront:app];
                            }];
    app.windowsMenu = m_windowMenu.menu;
}

void
CocoaApplicationMenuBuilder::createHelpMenu(NSMenu *bar, NSApplication *app)
{
    char buffer[Inline::kLongNameStackBufferSize];
    NSString *appName = [[NSProcessInfo processInfo] processName];
    NSMenuItem *helpMenuItem = [bar addItemWithTitle:@"" action:nil keyEquivalent:@""];
    m_helpMenu = [[MenuItemCollection alloc] initWithTitle:translatedString(kMenuItemTypeHelpTitle)];
    [m_helpMenu setParentMenu:helpMenuItem];
    const char *translated = stripMnemonic(buffer, sizeof(buffer), m_translator->translate("nanoem.menu.help.online"));
    NSString *title = [[NSString alloc] initWithUTF8String:translated];
    [m_helpMenu
        addItemWithTitle:title
                  action:^() {
                      NSString *format = [NSString
                          stringWithFormat:
                              @"https://nanoem.readthedocs.io/ja/latest/?utm_source=nanoem-%s&utm_medium=referral",
                          nanoemGetVersionString()];
                      NSURL *location = [NSURL URLWithString:format];
                      [[NSWorkspace sharedWorkspace] openURL:location];
                  }];
    [m_helpMenu addItemWithTitle:[[NSString alloc] initWithFormat:NSLocalizedString(@"%@ Help", nil), appName]
                          action:^() {
                              NSApplication *app = [NSApplication sharedApplication];
                              [app showHelp:app];
                          }
                   keyEquivalent:@"?"];
    [m_helpMenu addSeparator];
    [m_helpMenu addItemWithTitle:@"Open Redo Log Directory"
                          action:^() {
                              const char *redoPath = json_object_dotget_string(
                                  json_object(m_parent->service()->applicationConfiguration()), "macos.redo.path");
                              NSURL *redoDirectoryURL =
                                  [[NSURL alloc] initFileURLWithPath:[[NSString alloc] initWithUTF8String:redoPath]];
                              [[NSWorkspace sharedWorkspace] openURL:redoDirectoryURL];
                          }];
#if defined(NANOEM_HOCKEYSDK_APP_IDENTIFIER)
    [m_helpMenu addSeparator];
    [m_helpMenu addItemWithTitle:[[NSString alloc] initWithFormat:NSLocalizedString(@"%@ Feedback", nil), appName]
                          action:^() {
                              [[BITHockeyManager sharedHockeyManager].feedbackManager showFeedbackWindow];
                              m_tracker->send("osx", "nanoem.macos.help.feedback", "click", nullptr);
                          }];
#endif
    app.helpMenu = m_helpMenu.menu;
}

void
CocoaApplicationMenuBuilder::newProject()
{
    m_parent->clearTitle();
    m_client->sendNewProjectMessage();
}

void
CocoaApplicationMenuBuilder::loadFile(NSOpenPanel *panel, IFileManager::DialogType type)
{
    const URI fileURI(CocoaThreadedApplicationService::canonicalFileURI(panel.URL));
    NSString *title =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.load.title")];
    NSString *format =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.load.message")];
    NSString *message =
        [[NSString alloc] initWithFormat:@"%@: %@", format, panel.URL.path.stringByStandardizingPath.lastPathComponent];
    m_parent->openProgressWindow(title, message, 0, true);
    m_client->addCompleteLoadingFileEventListener(
        [](void *userData, const URI &fileURI, uint32_t /* type */, uint64_t /* ticks */) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            NSString *lastPathComponent = [[NSString alloc] initWithUTF8String:fileURI.lastPathComponentConstString()];
            self->m_parent->setTitle(lastPathComponent);
            self->m_parent->closeProgressWindow();
        },
        this, true);
    m_client->sendLoadFileMessage(fileURI, type);
}

void
CocoaApplicationMenuBuilder::saveFile(NSSavePanel *panel, IFileManager::DialogType type)
{
    saveFile(CocoaThreadedApplicationService::canonicalFileURI(panel.URL), type);
}

void
CocoaApplicationMenuBuilder::saveFile(const URI &fileURI, IFileManager::DialogType type)
{
    NSString *title =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.save.title")];
    NSString *format =
        [[NSString alloc] initWithUTF8String:m_translator->translate("nanoem.dialog.progress.save.message")];
    NSString *filename = [[NSString alloc] initWithUTF8String:fileURI.lastPathComponentConstString()];
    NSString *message = [[NSString alloc] initWithFormat:@"%@: %@ ...", format, filename];
    m_parent->openProgressWindow(title, message, 0, false);
    m_client->addCompleteSavingFileEventListener(
        [](void *userData, const URI & /* fileURI */, uint32_t /* type */, uint64_t /* ticks */) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            self->m_parent->closeProgressWindow();
        },
        this, true);
    m_client->sendSaveFileMessage(fileURI, type);
}

void
CocoaApplicationMenuBuilder::exportImage()
{
    m_client->addCompleteExportImageConfigurationEventListener(
        [](void *userData, const StringList &availableExtensions) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            if (!availableExtensions.empty()) {
                NSWindow *window = self->m_parent->nativeWindow();
                NSSavePanel *panel = createSavePanel(availableExtensions);
                [panel beginSheetModalForWindow:window
                              completionHandler:^(NSInteger result) {
                                  clearModalPanel(panel, window);
                                  URI fileURI;
                                  if (result == NSModalResponseOK) {
                                      fileURI = CocoaThreadedApplicationService::canonicalFileURI(panel.URL);
                                  }
                                  self->m_client->sendExecuteExportingImageMessage(fileURI);
                              }];
            }
            else {
                self->m_client->sendExecuteExportingImageMessage(URI());
            }
        },
        this, true);
    m_client->sendRequestExportImageConfigurationMessage();
}

void
CocoaApplicationMenuBuilder::exportVideo()
{
    m_client->addAvailableAllExportingVideoExtensionsEvent(
        [](void *userData, const StringList & /* extensions */) {
            auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
            self->m_client->addCompleteExportVideoConfigurationEventListener(
                [](void *userData, const StringList &availableExtensions) {
                    auto self = static_cast<CocoaApplicationMenuBuilder *>(userData);
                    if (!availableExtensions.empty()) {
                        NSWindow *window = self->m_parent->nativeWindow();
                        NSSavePanel *panel = createSavePanel(availableExtensions);
                        [panel beginSheetModalForWindow:window
                                      completionHandler:^(NSInteger result) {
                                          clearModalPanel(panel, window);
                                          URI fileURI;
                                          if (result == NSModalResponseOK) {
                                              fileURI = CocoaThreadedApplicationService::canonicalFileURI(panel.URL);
                                          }
                                          self->m_client->sendExecuteExportingVideoMessage(fileURI);
                                      }];
                    }
                    else {
                        self->m_client->sendExecuteExportingVideoMessage(URI());
                    }
                },
                self, true);
            self->m_client->sendRequestExportVideoConfigurationMessage();
        },
        this, true);
    m_client->sendLoadAllEncoderPluginsMessage(cachedAggregateAllPlugins());
}

NSString *
CocoaApplicationMenuBuilder::translatedString(const char *text) const
{
    return [[NSString alloc] initWithUTF8String:m_translator->translate(text)];
}

URIList
CocoaApplicationMenuBuilder::cachedAggregateAllPlugins()
{
    if (m_cachedPluginURIs.empty()) {
        aggregateAllPlugins(m_cachedPluginURIs);
    }
    return m_cachedPluginURIs;
}

} /* namespace macos */
} /* namespace nanoem */
