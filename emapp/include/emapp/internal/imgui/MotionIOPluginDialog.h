/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_IMGUI_MOTIONIOPLUGINDIALOG_H_
#define NANOEM_EMAPP_INTERNAL_IMGUI_MOTIONIOPLUGINDIALOG_H_

#include "emapp/internal/imgui/BaseIOPluginDialog.h"

namespace nanoem {
namespace internal {
namespace imgui {

struct MotionIOPluginDialog : BaseIOPluginDialog {
    MotionIOPluginDialog(int functionIndex, const ByteArray &input, plugin::MotionIOPlugin *plugin, Project *project,
        Nanoem__Application__Plugin__UIWindow *window, BaseApplicationService *application);

    void handleResponse(Project *project) NANOEM_DECL_OVERRIDE;
    void handleUIComponent(
        const char *id, const Nanoem__Application__Plugin__UIComponent *component) NANOEM_DECL_OVERRIDE;
    bool reloadUILayout(ByteArray &bytes, Error &error) NANOEM_DECL_OVERRIDE;

    const ByteArray m_input;
    plugin::MotionIOPlugin *m_plugin;
};

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_IMGUI_MOTIONIOPLUGINDIALOG_H_ */
