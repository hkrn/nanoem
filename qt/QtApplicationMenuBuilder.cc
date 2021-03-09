/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "QtApplicationMenuBuilder.h"

#include "MainWindow.h"
#include "emapp/Accessory.h"
#include "emapp/BaseApplicationClient.h"
#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/Motion.h"

#include <QtDebug>

#include <QAction>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>

namespace nanoem {
namespace qt {

QStringList
QtApplicationMenuBuilder::toNameFilter(const StringList &allowedExtensions)
{
    QStringList newAllowedExtensions;
    for (const auto &item : allowedExtensions) {
        newAllowedExtensions << (QString("*.") + QString(item.c_str()));
    }
    return newAllowedExtensions;
}

QtApplicationMenuBuilder::QtApplicationMenuBuilder(
    MainWindow *parent, BaseApplicationClient *client, const ITranslator *translator)
    : QObject()
    , ApplicationMenuBuilder(client, false)
    , m_translator(translator)
    , m_parent(parent)
{
}

QtApplicationMenuBuilder::~QtApplicationMenuBuilder()
{
}

void
QtApplicationMenuBuilder::newProject()
{
    m_client->sendNewProjectMessage();
}

void
QtApplicationMenuBuilder::openProject()
{
    QFileDialog *dialog = new QFileDialog(m_parent);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setNameFilters(QStringList() << "*.pmm"
                                         << "*.nma");
    connect(dialog, &QFileDialog::accepted, [=]() { loadFile(dialog, IFileManager::kDialogTypeOpenProject); });
    connect(dialog, &QFileDialog::rejected, [=]() { m_client->clearAllCompleteLoadingFileOnceEventListeners(); });
    connect(dialog, &QFileDialog::finished, [=](int) { m_parent->setFocus(); });
    dialog->open();
}

void
QtApplicationMenuBuilder::saveProject()
{
    m_client->sendGetProjectFileURIRequestMessage(
        [](void *userData, const URI &fileURI) {
            auto self = static_cast<QtApplicationMenuBuilder *>(userData);
            const String &pathExtension = fileURI.pathExtension();
            if (!fileURI.isEmpty() && pathExtension == String("nma")) {
                self->saveFile(fileURI, IFileManager::kDialogTypeSaveProjectFile);
            }
            else {
                self->saveProjectAs();
            }
        },
        this);
}

void
QtApplicationMenuBuilder::saveProjectAs()
{
    saveFile(QStringList() << "nma", IFileManager::kDialogTypeOpenProject);
}

void
QtApplicationMenuBuilder::createAllMenus()
{
    QMenuBar *menuBar = m_parent->menuBar();
    menuBar->clear();
    MainMenuBarHandle handle = reinterpret_cast<MainMenuBarHandle>(menuBar);
    initialize();
    createFileMenu();
    createEditMenu(handle);
    appendMenuSeparator(m_editMenu);
    appendMenuItem(m_editMenu, kMenuItemTypeEditPreference);
    createProjectMenu(handle);
    createCameraMenu(handle);
    createLightMenu(handle);
    createModelMenu(handle);
    createAccessoryMenu(handle);
    createWindowMenu();
    createHelpMenu();
    setMenuItemEnable(kMenuItemTypeFileExportModel, false);
    setMenuItemEnable(kMenuItemTypeFileExportModelMotion, false);
    setMenuItemEnable(kMenuItemTypeFileExportModelPose, false);
}

ApplicationMenuBuilder::MenuBarHandle
QtApplicationMenuBuilder::createMenuBar()
{
    QMenu *menu = new QMenu(m_parent);
    return reinterpret_cast<ApplicationMenuBuilder::MenuBarHandle>(menu);
}

ApplicationMenuBuilder::MenuBarHandle
QtApplicationMenuBuilder::createMenuBar(MenuItemType type)
{
    QMenu *menu = new QMenu(translatedString(type), m_parent);
    return reinterpret_cast<MenuBarHandle>(menu);
}

ApplicationMenuBuilder::MenuItemHandle
QtApplicationMenuBuilder::createMenuItem(MainMenuBarHandle menu)
{
    return reinterpret_cast<MenuItemHandle>(menu);
}

ApplicationMenuBuilder::MenuItemHandle
QtApplicationMenuBuilder::appendMenuItem(MenuBarHandle menu, MenuItemType type)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    QAction *action = new QAction(translatedString(type), this);
    connect(action, &QAction::triggered, [=]() { m_client->sendMenuActionMessage(type); });
    switch (type) {
    case kMenuItemTypeEditUndo: {
        action->setShortcut(QKeySequence::Undo);
        break;
    }
    case kMenuItemTypeEditRedo: {
        action->setShortcut(QKeySequence::Redo);
        break;
    }
    case kMenuItemTypeEditCopy: {
        action->setShortcut(QKeySequence::Copy);
        break;
    }
    case kMenuItemTypeEditPaste: {
        action->setShortcut(QKeySequence::Paste);
        break;
    }
    case kMenuItemTypeEditCut: {
        action->setShortcut(QKeySequence::Cut);
        break;
    }
    case kMenuItemTypeEditSelectAll: {
        action->setShortcut(QKeySequence::SelectAll);
        break;
    }
    default:
        break;
    }
    menuPtr->addAction(action);
    return reinterpret_cast<ApplicationMenuBuilder::MenuItemHandle>(action);
}

void
QtApplicationMenuBuilder::createSelectAccessoryMenuItem(
    ApplicationMenuBuilder::MenuBarHandle menu, uint16_t handle, const char *name)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    QAction *action = new QAction(name, this);
    connect(action, &QAction::triggered, [=]() { m_client->sendSetActiveAccessoryMessage(handle); });
    action->setData(QVariant(handle));
    menuPtr->addAction(action);
}

void
QtApplicationMenuBuilder::createSelectModelMenuItem(
    ApplicationMenuBuilder::MenuBarHandle menu, uint16_t handle, const char *name)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    QAction *action = new QAction(name, this);
    connect(action, &QAction::triggered, [=]() { m_client->sendSetActiveModelMessage(handle); });
    action->setData(QVariant(handle));
    menuPtr->addAction(action);
}

void
QtApplicationMenuBuilder::createSelectBoneMenuItem(ApplicationMenuBuilder::MenuBarHandle menu, const char *name)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    QAction *action = new QAction(name, this);
    connect(action, &QAction::triggered, [=]() { m_client->sendSetActiveModelBoneMessage(name); });
    menuPtr->addAction(action);
}

void
QtApplicationMenuBuilder::createSelectMorphMenuItem(
    ApplicationMenuBuilder::MenuBarHandle menu, nanoem_model_morph_category_t /* category */, const char *name)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    QAction *action = new QAction(name, this);
    connect(action, &QAction::triggered, [=]() { m_client->sendSetActiveModelMorphMessage(name, true); });
    menuPtr->addAction(action);
}

void
QtApplicationMenuBuilder::createPluginMenuItem(
    MenuBarHandle menu, MenuItemType type, uint16_t handle, const String &name, const StringList &items)
{
    switch (type) {
    case kMenuItemTypeModelPluginExecute: {
        if (auto menuPtr = reinterpret_cast<QMenu *>(menu)) {
            QMenu *baseMenu = new QMenu(name.c_str());
            nanoem_rsize_t offset = 0;
            for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it, ++offset) {
                const nanoem_rsize_t functionIndex = offset;
                QAction *action = new QAction(it->c_str(), this);
                connect(action, &QAction::triggered,
                    [=]() { m_client->sendExecuteModelPluginMessage(handle, functionIndex); });
                baseMenu->addAction(action);
            }
            menuPtr->addMenu(baseMenu);
        }
        break;
    }
    case kMenuItemTypeMotionPluginExecute: {
        if (auto menuPtr = reinterpret_cast<QMenu *>(menu)) {
            QMenu *baseMenu = new QMenu(name.c_str());
            nanoem_rsize_t offset = 0;
            for (StringList::const_iterator it = items.begin(), end = items.end(); it != end; ++it, ++offset) {
                const nanoem_rsize_t functionIndex = offset;
                QAction *action = new QAction(it->c_str(), this);
                connect(action, &QAction::triggered,
                    [=]() { m_client->sendExecuteMotionPluginMessage(handle, functionIndex); });
                baseMenu->addAction(action);
            }
            menuPtr->addMenu(baseMenu);
        }
        break;
    }
    default:
        break;
    }
}

void
QtApplicationMenuBuilder::updateAllSelectDrawableItems(ApplicationMenuBuilder::MenuBarHandle menu, uint16_t handle)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    for (auto item : menuPtr->actions()) {
        item->setChecked(item->data().toInt() == handle);
    }
}

void
QtApplicationMenuBuilder::removeMenuItemById(ApplicationMenuBuilder::MenuBarHandle menu, int index)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    const auto actions = menuPtr->actions();
    if (index >= 0 && actions.size() > index) {
        menuPtr->removeAction(actions.at(index));
    }
}

void
QtApplicationMenuBuilder::appendMenuSeparator(ApplicationMenuBuilder::MenuBarHandle menu)
{
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    menuPtr->addSeparator();
}

void
QtApplicationMenuBuilder::clearAllMenuItems(ApplicationMenuBuilder::MenuBarHandle menu)
{
    if (auto menuPtr = reinterpret_cast<QMenu *>(menu)) {
        menuPtr->clear();
    }
}

void
QtApplicationMenuBuilder::setParentMenu(
    ApplicationMenuBuilder::MenuItemHandle parent, ApplicationMenuBuilder::MenuBarHandle menu)
{
    auto objectPtr = reinterpret_cast<QObject *>(parent);
    auto menuPtr = reinterpret_cast<QMenu *>(menu);
    if (QMenuBar *parentPtr = qobject_cast<QMenuBar *>(objectPtr)) {
        parentPtr->addMenu(menuPtr);
    }
    else if (QAction *parentPtr = qobject_cast<QAction *>(objectPtr)) {
        parentPtr->setMenu(menuPtr);
    }
}

void
QtApplicationMenuBuilder::setMenuItemEnabled(ApplicationMenuBuilder::MenuItemHandle item, bool value)
{
    auto action = reinterpret_cast<QAction *>(item);
    action->setEnabled(value);
}

void
QtApplicationMenuBuilder::setMenuItemChecked(ApplicationMenuBuilder::MenuItemHandle item, bool value)
{
    auto action = reinterpret_cast<QAction *>(item);
    action->setChecked(value);
}

bool
QtApplicationMenuBuilder::isMenuItemEnabled(ApplicationMenuBuilder::MenuItemHandle item) const noexcept
{
    auto action = reinterpret_cast<const QAction *>(item);
    return action->isEnabled();
}

bool
QtApplicationMenuBuilder::isMenuItemChecked(ApplicationMenuBuilder::MenuItemHandle item) const noexcept
{
    auto action = reinterpret_cast<const QAction *>(item);
    return action->isChecked();
}

void
QtApplicationMenuBuilder::createFileMenu()
{
    QAction *action = nullptr;
    QMenu *menu = new QMenu(translatedString(kMenuItemTypeFileTitle));
    action = menu->addAction(translatedString(kMenuItemTypeFileNewProject));
    action->setShortcut(QKeySequence::New);
    connect(action, &QAction::triggered, [=]() { confirmBeforeNewProject(); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileOpenProject));
    action->setShortcut(QKeySequence::Open);
    connect(action, &QAction::triggered, [=]() { confirmBeforeOpeningProject(); });
    createFileImportMenu(menu);
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileSaveProject));
    action->setShortcut(QKeySequence::Save);
    connect(action, &QAction::triggered, [=]() { saveProject(); });
    action = menu->addAction(translatedString(kMenuItemTypeFileSaveAsProject));
    action->setShortcut(QKeySequence::SaveAs);
    connect(action, &QAction::triggered, [=]() { saveProjectAs(); });
    createFileExportMenu(menu);
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileExit));
    action->setShortcut(QKeySequence::Quit);
    connect(action, &QAction::triggered, qApp, &QApplication::closeAllWindows);
    m_parent->menuBar()->addMenu(menu);
}

void
QtApplicationMenuBuilder::createFileImportMenu(QMenu *parent)
{
    QAction *action = nullptr;
    QMenu *menu = parent->addMenu(translatedString(kMenuItemTypeFileImportTitle));
    action = menu->addAction(translatedString(kMenuItemTypeFileImportModel));
    connect(action, &QAction::triggered,
        [=]() { loadFile(Model::loadableExtensions(), IFileManager::kDialogTypeOpenModelFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileImportAccessory));
    connect(action, &QAction::triggered,
        [=]() { loadFile(Accessory::loadableExtensions(), IFileManager::kDialogTypeOpenModelFile); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileImportModelPose));
    connect(action, &QAction::triggered,
        [=]() { loadFile(QStringList() << "vpd", IFileManager::kDialogTypeOpenModelMotionFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileImportModelMotion));
    connect(action, &QAction::triggered,
        [=]() { loadFile(Motion::loadableExtensions(), IFileManager::kDialogTypeOpenModelMotionFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileImportCameraMotion));
    connect(action, &QAction::triggered,
        [=]() { loadFile(Motion::loadableExtensions(), IFileManager::kDialogTypeOpenCameraMotionFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileImportLightMotion));
    connect(action, &QAction::triggered,
        [=]() { loadFile(Motion::loadableExtensions(), IFileManager::kDialogTypeOpenLightMotionFile); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileImportAudio));
    connect(action, &QAction::triggered, [=]() {
        URIList plugins;
        // aggregateAllPlugins(plugins);
        m_client->addAvailableAllImportingAudioExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                self->loadFile(extensions, IFileManager::kDialogTypeOpenAudioFile);
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(plugins);
    });
    action = menu->addAction(translatedString(kMenuItemTypeFileImportVideo));
    connect(action, &QAction::triggered, [=]() {
        URIList plugins;
        // aggregateAllPlugins(plugins);
        m_client->addAvailableAllImportingAudioExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                self->loadFile(extensions, IFileManager::kDialogTypeOpenVideoFile);
            },
            this, true);
        m_client->sendLoadAllDecoderPluginsMessage(plugins);
    });
}

void
QtApplicationMenuBuilder::createFileExportMenu(QMenu *parent)
{
    QAction *action = nullptr;
    QMenu *menu = parent->addMenu(translatedString(kMenuItemTypeFileExportTitle));
    action = menu->addAction(translatedString(kMenuItemTypeFileExportModelPose));
    connect(action, &QAction::triggered,
        [=]() { saveFile(QStringList() << "vpd", IFileManager::kDialogTypeSaveModelMotionFile); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileExportModelMotion));
    connect(action, &QAction::triggered,
        [=]() { saveFile(Motion::loadableExtensions(), IFileManager::kDialogTypeSaveModelMotionFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileExportCameraMotion));
    connect(action, &QAction::triggered,
        [=]() { saveFile(Motion::loadableExtensions(), IFileManager::kDialogTypeSaveCameraMotionFile); });
    action = menu->addAction(translatedString(kMenuItemTypeFileExportLightMotion));
    connect(action, &QAction::triggered,
        [=]() { saveFile(Motion::loadableExtensions(), IFileManager::kDialogTypeSaveLightMotionFile); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileExportModel));
    connect(action, &QAction::triggered,
        [=]() { saveFile(Model::loadableExtensions(), IFileManager::kDialogTypeSaveModelFile); });
    menu->addSeparator();
    action = menu->addAction(translatedString(kMenuItemTypeFileExportImage));
    connect(action, &QAction::triggered, [=]() {
        QFileDialog *dialog = new QFileDialog(m_parent);
        dialog->setAcceptMode(QFileDialog::AcceptOpen);
        dialog->setFileMode(QFileDialog::ExistingFile);
        dialog->setNameFilters(QStringList() << "*.png"
                                             << "*.bmp"
                                             << "*.jpg"
                                             << "*.tga");
        connect(dialog, &QFileDialog::accepted, [=]() {
            // const QByteArray &filePath = dialog->selectedUrls().first().path().toUtf8();
            // m_client->sendExportImageMessage(URI::createFromFilePath(filePath.constData()), Vector2UI16());
        });
        connect(dialog, &QFileDialog::finished, [=](int) { m_parent->setFocus(); });
        dialog->open();
    });
    action = menu->addAction(translatedString(kMenuItemTypeFileExportVideo));
    connect(action, &QAction::triggered, [=]() {
        URIList plugins;
        // aggregateAllPlugins(plugins);
        m_client->addAvailableAllExportingVideoExtensionsEvent(
            [](void *userData, const StringList &extensions) {
                auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                QFileDialog *dialog = new QFileDialog(self->m_parent);
                dialog->setAcceptMode(QFileDialog::AcceptOpen);
                dialog->setFileMode(QFileDialog::ExistingFile);
                dialog->setNameFilters(toNameFilter(extensions));
                connect(dialog, &QFileDialog::accepted, [=]() {
                    const QByteArray &filePath = dialog->selectedUrls().first().path().toUtf8();
                    // self->m_client->sendExportVideoMessage(
                    //     URI::createFromFilePath(filePath.constData()), Vector2UI16());
                });
                connect(dialog, &QFileDialog::finished, [=](int) { self->m_parent->setFocus(); });
                dialog->open();
            },
            this, true);
        m_client->sendLoadAllEncoderPluginsMessage(plugins);
    });
}

void
QtApplicationMenuBuilder::createWindowMenu()
{
    QAction *action = nullptr;
    QMenu *menu = new QMenu(translatedString(kMenuItemTypeWindowTitle));
    action = menu->addAction(translatedString(kMenuItemTypeWindowMinimize));
    connect(action, &QAction::triggered, [=]() {});
    action = menu->addAction(translatedString(kMenuItemTypeWindowMaximize));
    connect(action, &QAction::triggered, [=]() {});
    action = menu->addAction(translatedString(kMenuItemTypeWindowFullscreen));
    action->setShortcut(QKeySequence::FullScreen);
    connect(action, &QAction::triggered, [=]() {});
    m_parent->menuBar()->addMenu(menu);
}

void
QtApplicationMenuBuilder::createHelpMenu()
{
    QAction *action = nullptr;
    QMenu *menu = new QMenu(translatedString(kMenuItemTypeHelpTitle), m_parent);
    action = menu->addAction(translatedString(kMenuItemTypeHelpAbout));
    connect(action, &QAction::triggered, [=]() { m_client->sendMenuActionMessage(kMenuItemTypeHelpAbout); });
    action = menu->addAction(translatedString(kMenuItemTypeHelpOnline));
    connect(action, &QAction::triggered, [=]() {
        QDesktopServices::openUrl(
            QString::asprintf("https://nanoem.readthedocs.io/ja/latest/?utm_source=nanoem-%s&utm_medium=referral",
                nanoemGetVersionString()));
    });
    m_parent->menuBar()->addMenu(menu);
}

void
QtApplicationMenuBuilder::confirmBeforeNewProject()
{
    m_client->sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<QtApplicationMenuBuilder *>(userData);
            BaseApplicationClient *client = self->m_client;
            if (dirty) {
                client->clearAllProjectAfterConfirmOnceEventListeners();
                client->addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                        self->m_client->addCompleteSavingFileEventListener(
                            [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
                                BX_UNUSED_3(fileURI, type, ticks);
                                auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                                self->newProject();
                            },
                            self, true);
                        self->saveProject();
                    },
                    self, true);
                client->addDiscardProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                        self->newProject();
                    },
                    self, true);
                client->sendConfirmBeforeNewProjectMessage();
            }
            else {
                self->newProject();
            }
        },
        this);
}

void
QtApplicationMenuBuilder::confirmBeforeOpeningProject()
{
    m_client->sendIsProjectDirtyRequestMessage(
        [](void *userData, bool dirty) {
            auto self = static_cast<QtApplicationMenuBuilder *>(userData);
            BaseApplicationClient *client = self->m_client;
            if (dirty) {
                client->clearAllProjectAfterConfirmOnceEventListeners();
                client->addSaveProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                        self->m_client->addCompleteSavingFileEventListener(
                            [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
                                BX_UNUSED_3(type, fileURI, ticks);
                                auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                                self->openProject();
                            },
                            self, true);
                        self->saveProject();
                    },
                    self, true);
                client->addDiscardProjectAfterConfirmEventListener(
                    [](void *userData) {
                        auto self = static_cast<QtApplicationMenuBuilder *>(userData);
                        self->openProject();
                    },
                    self, true);
                client->sendConfirmBeforeOpenProjectMessage();
            }
            else {
                self->openProject();
            }
        },
        this);
}

void
QtApplicationMenuBuilder::loadFile(const StringList &extensions, IFileManager::DialogType type)
{
    loadFile(toNameFilter(extensions), type);
}

void
QtApplicationMenuBuilder::saveFile(const StringList &extensions, IFileManager::DialogType type)
{
    saveFile(toNameFilter(extensions), type);
}

void
QtApplicationMenuBuilder::loadFile(const QStringList &extensions, IFileManager::DialogType type)
{
    QFileDialog *dialog = new QFileDialog(m_parent);
    dialog->setAcceptMode(QFileDialog::AcceptOpen);
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setNameFilters(extensions);
    connect(dialog, &QFileDialog::accepted, [=]() { loadFile(dialog, type); });
    connect(dialog, &QFileDialog::rejected, [=]() { m_client->clearAllCompleteLoadingFileOnceEventListeners(); });
    connect(dialog, &QFileDialog::finished, [=](int) { m_parent->setFocus(); });
    dialog->open();
}

void
QtApplicationMenuBuilder::saveFile(const QStringList &extensions, IFileManager::DialogType type)
{
    QFileDialog *dialog = new QFileDialog(m_parent);
    dialog->setAcceptMode(QFileDialog::AcceptSave);
    dialog->setNameFilters(extensions);
    connect(dialog, &QFileDialog::accepted, [=]() { saveFile(dialog, type); });
    connect(dialog, &QFileDialog::rejected, [=]() { m_client->clearAllCompleteSavingFileOnceEventListeners(); });
    connect(dialog, &QFileDialog::finished, [=](int) { m_parent->setFocus(); });
    dialog->open();
}

void
QtApplicationMenuBuilder::loadFile(const QFileDialog *dialog, IFileManager::DialogType type)
{
    const QByteArray &path = dialog->selectedUrls().first().path().toUtf8();
    loadFile(URI::createFromFilePath(path.constData()), type);
}

void
QtApplicationMenuBuilder::saveFile(const QFileDialog *dialog, IFileManager::DialogType type)
{
    const QByteArray &path = dialog->selectedUrls().first().path().toUtf8();
    saveFile(URI::createFromFilePath(path.constData()), type);
}

void
QtApplicationMenuBuilder::loadFile(const URI &fileURI, IFileManager::DialogType type)
{
    m_parent->openProgressDialog(m_translator->translate("nanoem.dialog->progress.load.title"),
        QString::asprintf("%s: %s...", m_translator->translate("nanoem.dialog->progress.load.message"),
            fileURI.lastPathComponent().c_str()));
    m_client->addCompleteLoadingFileEventListener(
        [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
            BX_UNUSED_3(fileURI, type, ticks);
            auto self = static_cast<QtApplicationMenuBuilder *>(userData);
            self->m_parent->closeProgressDialog();
        },
        this, true);
    m_client->sendLoadFileMessage(fileURI, type);
}

void
QtApplicationMenuBuilder::saveFile(const URI &fileURI, IFileManager::DialogType type)
{
    m_parent->openProgressDialog(m_translator->translate("nanoem.dialog->progress.save.title"),
        QString::asprintf("%s: %s...", m_translator->translate("nanoem.dialog->progress.save.message"),
            fileURI.lastPathComponent().c_str()));
    m_client->addCompleteSavingFileEventListener(
        [](void *userData, const URI &fileURI, uint32_t type, uint64_t ticks) {
            BX_UNUSED_3(fileURI, type, ticks);
            auto self = static_cast<QtApplicationMenuBuilder *>(userData);
            self->m_parent->closeProgressDialog();
        },
        this, true);
    m_client->sendSaveFileMessage(fileURI, type);
}

QString
QtApplicationMenuBuilder::translatedString(MenuItemType type) const
{
    return m_translator->translate(menuItemString(type));
}

} /* namespace qt */
} /* namespace nanoem */
