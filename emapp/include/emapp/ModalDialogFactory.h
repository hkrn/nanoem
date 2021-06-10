/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_MODALFACTORY_H_
#define NANOEM_EMAPP_MODALFACTORY_H_

#include "emapp/Forward.h"

namespace nanoem {

class Accessory;
class BaseApplicationService;
class IDelegate;
class IFileReader;
class IModalDialog;
class ITranslator;
class Model;
class Project;
class URI;

class ModalDialogFactory NANOEM_DECL_SEALED : private NonCopyable {
public:
    typedef IModalDialog *(*standard_confirm_dialog_t)(void *, Project *);
    struct AddingModelDialogCallback {
        typedef IModalDialog *(*adding_model_dialog_t)(void *, Model *, Project *);
        AddingModelDialogCallback(adding_model_dialog_t accepted, void *userData);
        adding_model_dialog_t m_accepted;
        void *m_userData;
    };
    struct DestroyReaderCallback {
        typedef void (*destroy_reader_t)(IFileReader *reader);
        DestroyReaderCallback(destroy_reader_t callback, IFileReader *reader);
        destroy_reader_t m_callback;
        IFileReader *m_reader;
    };
    struct EnablingEffectPluginDialogCallback {
        typedef IModalDialog *(*enabling_effect_plugin_dialog_t)(
            void *, const URI &, nanoem_u16_t handle, int, Project *);
        EnablingEffectPluginDialogCallback(enabling_effect_plugin_dialog_t accepted, void *userData);
        enabling_effect_plugin_dialog_t m_accepted;
        void *m_userData;
    };
    struct StandardConfirmDialogCallbackPair {
        StandardConfirmDialogCallbackPair(standard_confirm_dialog_t accepted, standard_confirm_dialog_t cancelled = 0);
        standard_confirm_dialog_t m_accepted;
        standard_confirm_dialog_t m_cancelled;
    };
    struct SaveConfirmDialog {
        SaveConfirmDialog(
            standard_confirm_dialog_t save, standard_confirm_dialog_t discard, standard_confirm_dialog_t cancel = 0);
        standard_confirm_dialog_t m_save;
        standard_confirm_dialog_t m_discard;
        standard_confirm_dialog_t m_cancel;
    };

    ~ModalDialogFactory() NANOEM_DECL_NOEXCEPT;

    static IModalDialog *createLoadingModelConfirmDialog(
        BaseApplicationService *applicationPtr, Model *model, AddingModelDialogCallback callback);
    static IModalDialog *createEnablingEffectPluginConfirmDialog(BaseApplicationService *applicationPtr,
        const URI &fileURI, nanoem_u16_t handle, int type, EnablingEffectPluginDialogCallback callback);
    static IModalDialog *createLoadingArchivedModelConfirmDialog(
        BaseApplicationService *applicationPtr, Model *model, const DestroyReaderCallback &reader);
    static IModalDialog *createDisplayPlainTextDialog(
        BaseApplicationService *applicationPtr, const String &title, const String &message);
    static IModalDialog *createStandardConfirmDialog(BaseApplicationService *applicationPtr, const String &title,
        const String &message, const StandardConfirmDialogCallbackPair &callbacks, void *userData);
    static IModalDialog *createConfirmSavingDialog(BaseApplicationService *applicationPtr, const String &title,
        const String &message, const SaveConfirmDialog &callbacks, void *userData);
    static IModalDialog *createConfirmConvertingAccessoryToModelDialog(
        Accessory *accessory, BaseApplicationService *applicationPtr);
    static IModalDialog *createConfirmDeletingAccessoryDialog(
        Accessory *accessory, BaseApplicationService *applicationPtr);
    static IModalDialog *createConfirmDeletingModelDialog(Model *model, BaseApplicationService *applicationPtr);
    static IModalDialog *createProgressDialog(BaseApplicationService *applicationPtr, const String &title,
        const String &message, const standard_confirm_dialog_t &callback, void *userData);
    static IModalDialog *createAboutDialog(BaseApplicationService *applicationPtr);
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_MODALFACTORY_H_ */
