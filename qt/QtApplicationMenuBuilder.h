/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_QT_QTAPPLICATIONMENUBUILDER_H_
#define NANOEM_EMAPP_QT_QTAPPLICATIONMENUBUILDER_H_

#include "emapp/ApplicationMenuBuilder.h"
#include "emapp/IFileManager.h"

#include <QObject>

class QFileDialog;
class QMenuBar;
class QMenu;

namespace nanoem {

class ITranslator;

namespace qt {

class MainWindow;

class QtApplicationMenuBuilder final : public QObject, public ApplicationMenuBuilder {
    Q_OBJECT
public:
    static QStringList toNameFilter(const StringList &allowedExtensions);

    QtApplicationMenuBuilder(MainWindow *parent, BaseApplicationClient *client, const ITranslator *translator);
    ~QtApplicationMenuBuilder() override;

    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();

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
    void removeMenuItemById(MenuBarHandle menu, int index) override;
    void appendMenuSeparator(MenuBarHandle menu) override;
    void clearAllMenuItems(MenuBarHandle menu) override;
    void setParentMenu(MenuItemHandle parent, MenuBarHandle menu) override;
    void setMenuItemEnabled(MenuItemHandle item, bool value) override;
    void setMenuItemChecked(MenuItemHandle item, bool value) override;
    bool isMenuItemEnabled(MenuItemHandle item) const noexcept override;
    bool isMenuItemChecked(MenuItemHandle item) const noexcept override;

    void createFileMenu();
    void createFileImportMenu(QMenu *parent);
    void createFileExportMenu(QMenu *parent);
    void createWindowMenu();
    void createHelpMenu();
    void confirmBeforeNewProject();
    void confirmBeforeOpeningProject();
    void loadFile(const StringList &extensions, IFileManager::DialogType type);
    void saveFile(const StringList &extensions, IFileManager::DialogType type);
    void loadFile(const QStringList &extensions, IFileManager::DialogType type);
    void saveFile(const QStringList &extensions, IFileManager::DialogType type);
    void loadFile(const QFileDialog *dialog, IFileManager::DialogType type);
    void saveFile(const QFileDialog *dialog, IFileManager::DialogType type);
    void loadFile(const URI &fileURI, IFileManager::DialogType type);
    void saveFile(const URI &fileURI, IFileManager::DialogType type);
    QString translatedString(MenuItemType type) const;

    const ITranslator *m_translator;
    MainWindow *m_parent;
};

} /* namespace qt */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_QT_QTAPPLICATIONMENUBUILDER_H_ */
