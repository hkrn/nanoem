/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ModalDialogFactory.h"

#include "emapp/Accessory.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IModalDialog.h"
#include "emapp/ITranslator.h"
#include "emapp/Model.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

#include "imgui/imgui.h"

namespace nanoem {
namespace {

class NoActionDialog : public IModalDialog {
protected:
    NoActionDialog(BaseApplicationService *applicationPtr);

    Vector4 desiredWindowSize(
        const Vector2UI16 &devicePixelWindowSize, nanoem_f32_t devicePxielRatio) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onSaved(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onDiscarded(Project *project) NANOEM_DECL_OVERRIDE;
    bool isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    bool isCancelled() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setProgress(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void setRowHeight(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void setTitle(const char *value) NANOEM_DECL_OVERRIDE;
    void setText(const char *value) NANOEM_DECL_OVERRIDE;

protected:
    BaseApplicationService *m_applicationPtr;
    nanoem_f32_t m_rowHeight;
    bool m_cancelled;
};

NoActionDialog::NoActionDialog(BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_rowHeight(0)
    , m_cancelled(false)
{
}

Vector4
NoActionDialog::desiredWindowSize(
    const Vector2UI16 &devicePixelWindowSize, nanoem_f32_t /* devicePxielRatio */) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_f32_t widthRatio = 0.65f, heightRatio = 0.65f;
    const nanoem_f32_t widthOffsetRatio = (1.0f - widthRatio) * 0.5f, heightOffsetRatio = (1.0f - heightRatio) * 0.5f;
    return Vector4(devicePixelWindowSize.x * widthOffsetRatio, devicePixelWindowSize.y * heightOffsetRatio,
        devicePixelWindowSize.x * widthRatio, devicePixelWindowSize.y * heightRatio);
}

IModalDialog *
NoActionDialog::onAccepted(Project *project)
{
    BX_UNUSED_1(project);
    return nullptr;
}

IModalDialog *
NoActionDialog::onCancelled(Project *project)
{
    BX_UNUSED_1(project);
    m_cancelled = true;
    return nullptr;
}

IModalDialog *
NoActionDialog::onSaved(Project *project)
{
    BX_UNUSED_1(project);
    return nullptr;
}

IModalDialog *
NoActionDialog::onDiscarded(Project *project)
{
    BX_UNUSED_1(project);
    return nullptr;
}

bool
NoActionDialog::isButtonEnabled(nanoem_u32_t value) const NANOEM_DECL_NOEXCEPT
{
    BX_UNUSED_1(value);
    return true;
}

bool
NoActionDialog::isCancelled() const NANOEM_DECL_NOEXCEPT
{
    return m_cancelled;
}

void
NoActionDialog::setProgress(nanoem_f32_t value)
{
    BX_UNUSED_1(value);
}

void
NoActionDialog::setRowHeight(nanoem_f32_t value)
{
    m_rowHeight = value;
}

void
NoActionDialog::setTitle(const char *value)
{
    BX_UNUSED_1(value);
}

void
NoActionDialog::setText(const char *value)
{
    BX_UNUSED_1(value);
}

class LoadingModelConfirmDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    LoadingModelConfirmDialog(
        BaseApplicationService *applicationPtr, Model *model, ModalDialogFactory::AddingModelDialogCallback callback);
    ~LoadingModelConfirmDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;

private:
    String m_message;
    ModalDialogFactory::AddingModelDialogCallback m_callback;
    String m_title;
    Model *m_model;
};

LoadingModelConfirmDialog::LoadingModelConfirmDialog(
    BaseApplicationService *applicationPtr, Model *model, ModalDialogFactory::AddingModelDialogCallback callback)
    : NoActionDialog(applicationPtr)
    , m_message(model->comment())
    , m_callback(callback)
    , m_model(model)
{
    StringUtils::format(m_title, applicationPtr->translator()->translate("nanoem.window.title.confirm.model"),
        model->canonicalNameConstString());
    if (m_message.empty()) {
        nanoem_unicode_string_factory_t *factory = model->project()->unicodeStringFactory();
        StringUtils::getUtf8String(
            nanoemModelGetComment(model->data(), NANOEM_LANGUAGE_TYPE_FIRST_ENUM), factory, m_message);
    }
}

LoadingModelConfirmDialog::~LoadingModelConfirmDialog() NANOEM_DECL_NOEXCEPT
{
    if (m_model) {
        Project *project = m_model->project();
        project->destroyModel(m_model);
        m_model = nullptr;
    }
}

const char *
LoadingModelConfirmDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
LoadingModelConfirmDialog::draw(const Project *)
{
    MutableString s(m_message.c_str(), m_message.c_str() + m_message.size());
    s.push_back(0);
    ImVec2 size(ImGui::GetContentRegionAvail());
    size.y -= ImGui::GetFrameHeightWithSpacing();
    ImGui::InputTextMultiline("##text", s.data(), s.size(), size, ImGuiInputTextFlags_ReadOnly);
}

nanoem_u32_t
LoadingModelConfirmDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk | kButtonTypeCancel;
}

IModalDialog *
LoadingModelConfirmDialog::onAccepted(Project *project)
{
    SG_PUSH_GROUPF("LoadingModelConfirmDialog::onAccepted(model=%s)", m_model->nameConstString());
    IModalDialog *nextDialog = nullptr;
    Error error;
    m_model->setupAllBindings();
    Progress progress(project, m_model->createAllImages());
    m_model->upload();
    m_model->loadAllImages(progress, error);
    if (m_callback.m_accepted) {
        nextDialog = m_callback.m_accepted(m_callback.m_userData, m_model, project);
    }
    bool cancelled = error.isCancelled();
    if (!cancelled) {
        project->addModel(m_model);
        project->performModelSkinDeformer(m_model);
        project->setActiveModel(m_model);
        m_model->writeLoadCommandMessage(error);
        m_model->setVisible(true);
    }
    else {
        project->destroyModel(m_model);
    }
    m_model = nullptr;
    if (!nextDialog && error.hasReason()) {
        nextDialog = error.createModalDialog(m_applicationPtr);
        error.notify(project->eventPublisher());
    }
    progress.complete();
    SG_POP_GROUP();
    return nextDialog;
}

class EnablingEffectPluginConfirmDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    EnablingEffectPluginConfirmDialog(BaseApplicationService *applicationPtr, const URI &fileURI, nanoem_u16_t handle,
        int type, ModalDialogFactory::EnablingEffectPluginDialogCallback callback);
    ~EnablingEffectPluginConfirmDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;

private:
    const URI m_fileURI;
    const String m_title;
    const String m_message;
    const nanoem_u16_t m_handle;
    const int m_type;
    ModalDialogFactory::EnablingEffectPluginDialogCallback m_callback;
};

EnablingEffectPluginConfirmDialog::EnablingEffectPluginConfirmDialog(BaseApplicationService *applicationPtr,
    const URI &fileURI, nanoem_u16_t handle, int type, ModalDialogFactory::EnablingEffectPluginDialogCallback callback)
    : NoActionDialog(applicationPtr)
    , m_fileURI(fileURI)
    , m_title(applicationPtr->translator()->translate("nanoem.window.dialog.enabling-effect-plugin.title"))
    , m_message(applicationPtr->translator()->translate("nanoem.window.dialog.enabling-effect-plugin.message"))
    , m_handle(handle)
    , m_type(type)
    , m_callback(callback)
{
}

EnablingEffectPluginConfirmDialog::~EnablingEffectPluginConfirmDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
EnablingEffectPluginConfirmDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
EnablingEffectPluginConfirmDialog::draw(const Project *)
{
    ImGui::PushItemWidth(-1);
    ImGui::PushTextWrapPos();
    ImGui::TextUnformatted(m_message.c_str(), m_message.c_str() + m_message.size());
    ImGui::PopTextWrapPos();
    ImGui::PopItemWidth();
}

nanoem_u32_t
EnablingEffectPluginConfirmDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk | kButtonTypeCancel;
}

IModalDialog *
EnablingEffectPluginConfirmDialog::onAccepted(Project *project)
{
    IModalDialog *nextDialog = nullptr;
    project->setEffectPluginEnabled(true);
    if (m_callback.m_accepted) {
        nextDialog = m_callback.m_accepted(m_callback.m_userData, m_fileURI, m_handle, m_type, project);
    }
    return nextDialog;
}

class LoadingArchivedModelConfirmDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    LoadingArchivedModelConfirmDialog(BaseApplicationService *applicationPtr, Model *model,
        const ModalDialogFactory::DestroyReaderCallback &callback);
    ~LoadingArchivedModelConfirmDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;

private:
    const String m_message;
    ModalDialogFactory::DestroyReaderCallback m_callback;
    String m_title;
    Model *m_model;
};

LoadingArchivedModelConfirmDialog::LoadingArchivedModelConfirmDialog(
    BaseApplicationService *applicationPtr, Model *model, const ModalDialogFactory::DestroyReaderCallback &callback)
    : NoActionDialog(applicationPtr)
    , m_message(model->comment())
    , m_callback(callback)
    , m_model(model)
{
    StringUtils::format(m_title, applicationPtr->translator()->translate("nanoem.window.title.confirm.model"),
        model->nameConstString());
}

LoadingArchivedModelConfirmDialog::~LoadingArchivedModelConfirmDialog() NANOEM_DECL_NOEXCEPT
{
    if (m_model) {
        Project *project = m_model->project();
        project->destroyModel(m_model);
        m_model = nullptr;
        if (m_callback.m_callback) {
            m_callback.m_callback(m_callback.m_reader);
            m_callback.m_reader = nullptr;
        }
    }
}

const char *
LoadingArchivedModelConfirmDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
LoadingArchivedModelConfirmDialog::draw(const Project *)
{
    ImGui::PushItemWidth(-1);
    ImGui::PushTextWrapPos();
    ImGui::TextUnformatted(m_message.c_str(), m_message.c_str() + m_message.size());
    ImGui::PopTextWrapPos();
    ImGui::PopItemWidth();
}

nanoem_u32_t
LoadingArchivedModelConfirmDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk | kButtonTypeCancel;
}

IModalDialog *
LoadingArchivedModelConfirmDialog::onAccepted(Project *project)
{
    SG_PUSH_GROUPF("LoadingArchivedModelConfirmDialog::onAccepted(model=%s)", m_model->nameConstString());
    Progress progress(project, 0);
    Error error;
    if (m_model->uploadArchive(m_callback.m_reader, progress, error)) {
        progress.complete();
        project->addModel(m_model);
        project->performModelSkinDeformer(m_model);
        project->setActiveModel(m_model);
        m_model->setVisible(true);
        m_model = nullptr;
        m_callback.m_callback(m_callback.m_reader);
        m_callback.m_reader = nullptr;
    }
    else {
        progress.complete();
        error.notify(project->eventPublisher());
    }
    SG_POP_GROUP();
    return nullptr;
}

class DisplayPlainTextDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    DisplayPlainTextDialog(BaseApplicationService *applicationPtr, const String &title, const String &message);
    ~DisplayPlainTextDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    const String m_title;
    const String m_message;
};

DisplayPlainTextDialog::DisplayPlainTextDialog(
    BaseApplicationService *applicationPtr, const String &title, const String &message)
    : NoActionDialog(applicationPtr)
    , m_title(title)
    , m_message(message)
{
}

DisplayPlainTextDialog::~DisplayPlainTextDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
DisplayPlainTextDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
DisplayPlainTextDialog::draw(const Project *)
{
    MutableString s(m_message.c_str(), m_message.c_str() + m_message.size());
    s.push_back(0);
    ImVec2 size(ImGui::GetContentRegionAvail());
    size.y -= ImGui::GetFrameHeightWithSpacing();
    ImGui::InputTextMultiline("##text", s.data(), s.size(), size, ImGuiInputTextFlags_ReadOnly);
}

nanoem_u32_t
DisplayPlainTextDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk;
}

class StandardConfirmDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    StandardConfirmDialog(BaseApplicationService *applicationPtr, const String &title, const String &message,
        const ModalDialogFactory::StandardConfirmDialogCallbackPair &callbacks, void *userData);
    ~StandardConfirmDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onAccepted(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;

private:
    const String m_title;
    const String m_message;
    ModalDialogFactory::StandardConfirmDialogCallbackPair m_callbacks;
    void *m_userData;
};

StandardConfirmDialog::StandardConfirmDialog(BaseApplicationService *applicationPtr, const String &title,
    const String &message, const ModalDialogFactory::StandardConfirmDialogCallbackPair &callbacks, void *userData)
    : NoActionDialog(applicationPtr)
    , m_title(title)
    , m_message(message)
    , m_callbacks(callbacks)
    , m_userData(userData)
{
}

StandardConfirmDialog::~StandardConfirmDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
StandardConfirmDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
StandardConfirmDialog::draw(const Project *)
{
    ImGui::PushItemWidth(-1);
    ImGui::PushTextWrapPos();
    ImGui::TextUnformatted(m_message.c_str(), m_message.c_str() + m_message.size());
    ImGui::PopTextWrapPos();
    ImGui::PopItemWidth();
}

nanoem_u32_t
StandardConfirmDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk | kButtonTypeCancel;
}

IModalDialog *
StandardConfirmDialog::onAccepted(Project *project)
{
    IModalDialog *dialog = nullptr;
    if (ModalDialogFactory::standard_confirm_dialog_t callback = m_callbacks.m_accepted) {
        dialog = callback(m_userData, project);
    }
    return dialog;
}

IModalDialog *
StandardConfirmDialog::onCancelled(Project *project)
{
    IModalDialog *dialog = nullptr;
    if (ModalDialogFactory::standard_confirm_dialog_t callback = m_callbacks.m_cancelled) {
        dialog = callback(m_userData, project);
    }
    return dialog;
}

class ConfirmSavingDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    ConfirmSavingDialog(BaseApplicationService *applicationPtr, const String &title, const String &message,
        const ModalDialogFactory::SaveConfirmDialog &callbacks, void *userData);
    ~ConfirmSavingDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onSaved(Project *project) NANOEM_DECL_OVERRIDE;
    IModalDialog *onDiscarded(Project *project) NANOEM_DECL_OVERRIDE;

private:
    const String m_title;
    const String m_message;
    ModalDialogFactory::SaveConfirmDialog m_callbacks;
    void *m_userData;
};

ConfirmSavingDialog::ConfirmSavingDialog(BaseApplicationService *applicationPtr, const String &title,
    const String &message, const ModalDialogFactory::SaveConfirmDialog &callbacks, void *userData)
    : NoActionDialog(applicationPtr)
    , m_title(title)
    , m_message(message)
    , m_callbacks(callbacks)
    , m_userData(userData)
{
}

ConfirmSavingDialog::~ConfirmSavingDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
ConfirmSavingDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
ConfirmSavingDialog::draw(const Project *)
{
    ImGui::PushItemWidth(-1);
    ImGui::PushTextWrapPos();
    ImGui::TextUnformatted(m_message.c_str(), m_message.c_str() + m_message.size());
    ImGui::PopTextWrapPos();
    ImGui::PopItemWidth();
}

nanoem_u32_t
ConfirmSavingDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeSave | kButtonTypeDiscard | kButtonTypeCancel;
}

IModalDialog *
ConfirmSavingDialog::onCancelled(Project *project)
{
    IModalDialog *dialog = nullptr;
    if (ModalDialogFactory::standard_confirm_dialog_t callback = m_callbacks.m_cancel) {
        dialog = callback(m_userData, project);
    }
    return dialog;
}

IModalDialog *
ConfirmSavingDialog::onSaved(Project *project)
{
    IModalDialog *dialog = nullptr;
    if (ModalDialogFactory::standard_confirm_dialog_t callback = m_callbacks.m_save) {
        dialog = callback(m_userData, project);
    }
    return dialog;
}

IModalDialog *
ConfirmSavingDialog::onDiscarded(Project *project)
{
    IModalDialog *dialog = nullptr;
    if (ModalDialogFactory::standard_confirm_dialog_t callback = m_callbacks.m_discard) {
        dialog = callback(m_userData, project);
    }
    return dialog;
}

struct ConvertingAccessoryConfirmDialogUserData {
    ConvertingAccessoryConfirmDialogUserData(Accessory *accessory, BaseApplicationService *applicationPtr);

    static IModalDialog *onAccepted(void *userData, Project *project);
    static IModalDialog *onCancelled(void *userData, Project *project);

    BaseApplicationService *m_applicationPtr;
    Accessory *m_accessory;
};

ConvertingAccessoryConfirmDialogUserData::ConvertingAccessoryConfirmDialogUserData(
    Accessory *accessory, BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_accessory(accessory)
{
}

IModalDialog *
ConvertingAccessoryConfirmDialogUserData::onAccepted(void *userData, Project *project)
{
    ConvertingAccessoryConfirmDialogUserData *self = static_cast<ConvertingAccessoryConfirmDialogUserData *>(userData);
    if (!project->isPlaying()) {
        Error error;
        project->convertAccessoryToModel(self->m_accessory, error);
        error.notify(self->m_applicationPtr->eventPublisher());
    }
    nanoem_delete(self);
    return nullptr;
}

IModalDialog *
ConvertingAccessoryConfirmDialogUserData::onCancelled(void *userData, Project *project)
{
    BX_UNUSED_1(project);
    nanoem_delete(static_cast<ConvertingAccessoryConfirmDialogUserData *>(userData));
    return nullptr;
}

struct DeletingAccessoryConfirmDialogUserData {
    DeletingAccessoryConfirmDialogUserData(Accessory *accessory, BaseApplicationService *applicationPtr);

    static IModalDialog *onAccepted(void *userData, Project *project);
    static IModalDialog *onCancelled(void *userData, Project *project);

    BaseApplicationService *m_applicationPtr;
    Accessory *m_accessory;
};

DeletingAccessoryConfirmDialogUserData::DeletingAccessoryConfirmDialogUserData(
    Accessory *accessory, BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_accessory(accessory)
{
}

IModalDialog *
DeletingAccessoryConfirmDialogUserData::onAccepted(void *userData, Project *project)
{
    DeletingAccessoryConfirmDialogUserData *self = static_cast<DeletingAccessoryConfirmDialogUserData *>(userData);
    if (!project->isPlaying()) {
        Error error;
        Accessory *accessory = self->m_accessory;
        project->removeAccessory(accessory);
        accessory->writeDeleteCommandMessage(error);
        project->destroyAccessory(accessory);
        error.notify(self->m_applicationPtr->eventPublisher());
    }
    nanoem_delete(self);
    return nullptr;
}

IModalDialog *
DeletingAccessoryConfirmDialogUserData::onCancelled(void *userData, Project *project)
{
    BX_UNUSED_1(project);
    nanoem_delete(static_cast<DeletingAccessoryConfirmDialogUserData *>(userData));
    return nullptr;
}

struct DeletingModelConfirmDialogUserData {
    DeletingModelConfirmDialogUserData(Model *model, BaseApplicationService *applicationPtr);

    static IModalDialog *onAccepted(void *userData, Project *project);
    static IModalDialog *onCancelled(void *userData, Project *project);

    BaseApplicationService *m_applicationPtr;
    Model *m_model;
};

DeletingModelConfirmDialogUserData::DeletingModelConfirmDialogUserData(
    Model *model, BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_model(model)
{
}

IModalDialog *
DeletingModelConfirmDialogUserData::onAccepted(void *userData, Project *project)
{
    DeletingModelConfirmDialogUserData *self = static_cast<DeletingModelConfirmDialogUserData *>(userData);
    if (!project->isPlaying()) {
        Error error;
        Model *model = self->m_model;
        project->removeModel(model);
        model->writeDeleteCommandMessage(error);
        project->destroyModel(model);
        error.notify(self->m_applicationPtr->eventPublisher());
    }
    nanoem_delete(self);
    return nullptr;
}

IModalDialog *
DeletingModelConfirmDialogUserData::onCancelled(void *userData, Project *project)
{
    BX_UNUSED_1(project);
    nanoem_delete(static_cast<DeletingModelConfirmDialogUserData *>(userData));
    return nullptr;
}

class ProgressDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    ProgressDialog(BaseApplicationService *applicationPtr, const String &title, const String &message,
        const ModalDialogFactory::standard_confirm_dialog_t &callback, void *userData);
    ~ProgressDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    Vector4 desiredWindowSize(
        const Vector2UI16 &devicePixelWindowSize, nanoem_f32_t devicePxielRatio) const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    IModalDialog *onCancelled(Project *project) NANOEM_DECL_OVERRIDE;
    void setProgress(nanoem_f32_t value) NANOEM_DECL_OVERRIDE;
    void setTitle(const char *value) NANOEM_DECL_OVERRIDE;
    void setText(const char *value) NANOEM_DECL_OVERRIDE;

private:
    String m_title;
    String m_message;
    ModalDialogFactory::standard_confirm_dialog_t m_callback;
    nanoem_f32_t m_value;
    void *m_userData;
};

ProgressDialog::ProgressDialog(BaseApplicationService *applicationPtr, const String &title, const String &message,
    const ModalDialogFactory::standard_confirm_dialog_t &callback, void *userData)
    : NoActionDialog(applicationPtr)
    , m_title(title)
    , m_message(message)
    , m_callback(callback)
    , m_value(0)
    , m_userData(userData)
{
}

ProgressDialog::~ProgressDialog() NANOEM_DECL_NOEXCEPT
{
}

Vector4
ProgressDialog::desiredWindowSize(
    const Vector2UI16 &devicePixelWindowSize, nanoem_f32_t devicePxielRatio) const NANOEM_DECL_NOEXCEPT
{
    const nanoem_f32_t width = 540 * devicePxielRatio, height = ImGui::GetTextLineHeightWithSpacing() * 6;
    return Vector4((devicePixelWindowSize.x - width) * 0.5f, (devicePixelWindowSize.y - height) * 0.5f, width, height);
}

const char *
ProgressDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
ProgressDialog::draw(const Project *)
{
    ImGui::PushItemWidth(-1);
    ImGui::PushTextWrapPos();
    ImGui::TextUnformatted(m_message.c_str(), m_message.c_str() + m_message.size());
    ImGui::ProgressBar(m_value);
    ImGui::PopTextWrapPos();
    ImGui::PopItemWidth();
}

nanoem_u32_t
ProgressDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeCancel;
}

IModalDialog *
ProgressDialog::onCancelled(Project *project)
{
    return m_callback(m_userData, project);
}

void
ProgressDialog::setProgress(nanoem_f32_t value)
{
    m_value = glm::clamp(value, 0.0f, 1.0f);
}

void
ProgressDialog::setTitle(const char *value)
{
    m_title = value;
}

void
ProgressDialog::setText(const char *value)
{
    m_message = value;
}

class AboutDialog NANOEM_DECL_SEALED : public NoActionDialog {
public:
    AboutDialog(BaseApplicationService *applicationPtr);
    ~AboutDialog() NANOEM_DECL_NOEXCEPT_OVERRIDE;

    const char *title() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void draw(const Project *project) NANOEM_DECL_OVERRIDE;
    nanoem_u32_t buttons() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    String m_title;
};

AboutDialog::AboutDialog(BaseApplicationService *applicationPtr)
    : NoActionDialog(applicationPtr)
{
    StringUtils::format(m_title, "About nanoem %s", nanoemGetVersionString());
}

AboutDialog::~AboutDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
AboutDialog::title() const NANOEM_DECL_NOEXCEPT
{
    return m_title.c_str();
}

void
AboutDialog::draw(const Project *)
{
    static const char kPrefixText[] = "Copyright (c) 2015-2021 hkrn All rights reserved\n\n";
    MutableString text;
    text.insert(text.end(), kPrefixText, kPrefixText + sizeof(kPrefixText));
    const nanoem_u8_t *data = nullptr;
    size_t length = 0;
    resources::getCredits(data, length);
    text.insert(text.end() - 1, reinterpret_cast<const char *>(data), reinterpret_cast<const char *>(data + length));
    text.push_back(0);
    ImVec2 size(ImGui::GetContentRegionAvail());
    size.y -= ImGui::GetFrameHeightWithSpacing();
    ImGui::InputTextMultiline("##text", text.data(), text.size(), size, ImGuiInputTextFlags_ReadOnly);
}

nanoem_u32_t
AboutDialog::buttons() const NANOEM_DECL_NOEXCEPT
{
    return kButtonTypeOk;
}

} /* namespace anonymous */

ModalDialogFactory::AddingModelDialogCallback::AddingModelDialogCallback(adding_model_dialog_t accepted, void *userData)
    : m_accepted(accepted)
    , m_userData(userData)
{
}

ModalDialogFactory::DestroyReaderCallback::DestroyReaderCallback(destroy_reader_t callback, IFileReader *reader)
    : m_callback(callback)
    , m_reader(reader)
{
}

ModalDialogFactory::EnablingEffectPluginDialogCallback::EnablingEffectPluginDialogCallback(
    enabling_effect_plugin_dialog_t accepted, void *userData)
    : m_accepted(accepted)
    , m_userData(userData)
{
}

ModalDialogFactory::StandardConfirmDialogCallbackPair::StandardConfirmDialogCallbackPair(
    standard_confirm_dialog_t a, standard_confirm_dialog_t c)
    : m_accepted(a)
    , m_cancelled(c)
{
}

ModalDialogFactory::SaveConfirmDialog::SaveConfirmDialog(
    standard_confirm_dialog_t save, standard_confirm_dialog_t discard, standard_confirm_dialog_t cancel)
    : m_save(save)
    , m_discard(discard)
    , m_cancel(cancel)
{
}

IModalDialog *
ModalDialogFactory::createLoadingModelConfirmDialog(
    BaseApplicationService *applicationPtr, Model *model, AddingModelDialogCallback callback)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    nanoem_parameter_assert(model, "must NOT be nullptr");
    return nanoem_new(LoadingModelConfirmDialog(applicationPtr, model, callback));
}

IModalDialog *
ModalDialogFactory::createEnablingEffectPluginConfirmDialog(BaseApplicationService *applicationPtr, const URI &fileURI,
    nanoem_u16_t handle, int type, EnablingEffectPluginDialogCallback callback)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    return nanoem_new(EnablingEffectPluginConfirmDialog(applicationPtr, fileURI, handle, type, callback));
}

IModalDialog *
ModalDialogFactory::createLoadingArchivedModelConfirmDialog(
    BaseApplicationService *applicationPtr, Model *model, const DestroyReaderCallback &reader)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    nanoem_parameter_assert(model, "must NOT be nullptr");
    return nanoem_new(LoadingArchivedModelConfirmDialog(applicationPtr, model, reader));
}

IModalDialog *
ModalDialogFactory::createDisplayPlainTextDialog(
    BaseApplicationService *applicationPtr, const String &title, const String &message)
{
    return nanoem_new(DisplayPlainTextDialog(applicationPtr, title, message));
}

IModalDialog *
ModalDialogFactory::createStandardConfirmDialog(BaseApplicationService *applicationPtr, const String &title,
    const String &message, const StandardConfirmDialogCallbackPair &callbacks, void *userData)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    return nanoem_new(StandardConfirmDialog(applicationPtr, title, message, callbacks, userData));
}

IModalDialog *
ModalDialogFactory::createConfirmSavingDialog(BaseApplicationService *applicationPtr, const String &title,
    const String &message, const SaveConfirmDialog &callbacks, void *userData)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    return nanoem_new(ConfirmSavingDialog(applicationPtr, title, message, callbacks, userData));
}

IModalDialog *
ModalDialogFactory::createConfirmConvertingAccessoryToModelDialog(Accessory *accessory, BaseApplicationService *applicationPtr)
{
    nanoem_parameter_assert(accessory, "must NOT be nullptr");
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    ConvertingAccessoryConfirmDialogUserData *self =
        nanoem_new(ConvertingAccessoryConfirmDialogUserData(accessory, applicationPtr));
    const ModalDialogFactory::StandardConfirmDialogCallbackPair pair(
        ConvertingAccessoryConfirmDialogUserData::onAccepted, ConvertingAccessoryConfirmDialogUserData::onCancelled);
    const ITranslator *translator = applicationPtr->translator();
    return ModalDialogFactory::createStandardConfirmDialog(applicationPtr,
        translator->translate("nanoem.window.dialog.converting-accessory.title"),
        translator->translate("nanoem.window.dialog.converting-accessory.message"), pair, self);
}

IModalDialog *
ModalDialogFactory::createConfirmDeletingAccessoryDialog(Accessory *accessory, BaseApplicationService *applicationPtr)
{
    nanoem_parameter_assert(accessory, "must NOT be nullptr");
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    DeletingAccessoryConfirmDialogUserData *self =
        nanoem_new(DeletingAccessoryConfirmDialogUserData(accessory, applicationPtr));
    const ModalDialogFactory::StandardConfirmDialogCallbackPair pair(
        DeletingAccessoryConfirmDialogUserData::onAccepted, DeletingAccessoryConfirmDialogUserData::onCancelled);
    const ITranslator *translator = applicationPtr->translator();
    return ModalDialogFactory::createStandardConfirmDialog(applicationPtr,
        translator->translate("nanoem.window.dialog.deleting-accessory.title"),
        translator->translate("nanoem.window.dialog.deleting-accessory.message"), pair, self);
}

IModalDialog *
ModalDialogFactory::createConfirmDeletingModelDialog(Model *model, BaseApplicationService *applicationPtr)
{
    nanoem_parameter_assert(model, "must NOT be nullptr");
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    DeletingModelConfirmDialogUserData *self = nanoem_new(DeletingModelConfirmDialogUserData(model, applicationPtr));
    const ModalDialogFactory::StandardConfirmDialogCallbackPair pair(
        DeletingModelConfirmDialogUserData::onAccepted, DeletingModelConfirmDialogUserData::onCancelled);
    const ITranslator *translator = applicationPtr->translator();
    return ModalDialogFactory::createStandardConfirmDialog(applicationPtr,
        translator->translate("nanoem.window.dialog.deleting-model.title"),
        translator->translate("nanoem.window.dialog.deleting-model.message"), pair, self);
}

IModalDialog *
ModalDialogFactory::createProgressDialog(BaseApplicationService *applicationPtr, const String &title,
    const String &message, const standard_confirm_dialog_t &callback, void *userData)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    return nanoem_new(ProgressDialog(applicationPtr, title, message, callback, userData));
}

IModalDialog *
ModalDialogFactory::createAboutDialog(BaseApplicationService *applicationPtr)
{
    nanoem_parameter_assert(applicationPtr, "must NOT be nullptr");
    return nanoem_new(AboutDialog(applicationPtr));
}

ModalDialogFactory::~ModalDialogFactory() NANOEM_DECL_NOEXCEPT
{
}

} /* namespace nanoem */
