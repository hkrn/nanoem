/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_BASEIOPLUGINDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_BASEIOPLUGINDIALOG_H_

#include "emapp/Error.h"
#include "emapp/internal/PluginUI.h"
#include "emapp/internal/imgui/BaseNonModalDialogWindow.h"

struct Nanoem__Application__Plugin__UIComponent;
struct Nanoem__Application__Plugin__UIWindow;

namespace nanoem {
namespace internal {
namespace imgui {

struct BaseIOPluginDialog : BaseNonModalDialogWindow, PluginUI::IReactor {
    BaseIOPluginDialog(int functionIndex, const char *name, Project *project,
        Nanoem__Application__Plugin__UIWindow *window, BaseApplicationService *application);
    ~BaseIOPluginDialog() NANOEM_DECL_NOEXCEPT;

    virtual void handleResponse(Project *project) = 0;

    const char *windowId() NANOEM_DECL_NOEXCEPT;
    bool draw(Project *project) NANOEM_DECL_OVERRIDE;

    Project *m_project;
    PluginUI m_pluginUI;
    Error m_error;
    String m_windowId;
    int m_functionIndex;
    bool m_reloadLayout;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_BASEIOPLUGINDIALOG_H_ */
