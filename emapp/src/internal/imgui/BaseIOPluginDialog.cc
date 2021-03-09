/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/BaseIOPluginDialog.h"

#include "../../protoc/plugin.pb-c.h"

namespace nanoem {
namespace internal {
namespace imgui {

BaseIOPluginDialog::BaseIOPluginDialog(int functionIndex, const char *name, Project *project,
    Nanoem__Application__Plugin__UIWindow *window, BaseApplicationService *application)
    : BaseNonModalDialogWindow(application)
    , m_project(project)
    , m_pluginUI(this, window, name, project->windowDevicePixelRatio())
    , m_windowId(window->id)
    , m_functionIndex(functionIndex)
    , m_reloadLayout(false)
{
}

BaseIOPluginDialog::~BaseIOPluginDialog() NANOEM_DECL_NOEXCEPT
{
}

const char *
BaseIOPluginDialog::windowId() NANOEM_DECL_NOEXCEPT
{
    return m_windowId.c_str();
}

bool
BaseIOPluginDialog::draw(Project *project)
{
    BX_UNUSED_1(project);
    bool visible = true;
    if (m_reloadLayout) {
        m_pluginUI.reload(m_error);
        m_reloadLayout = false;
    }
    if (open(m_pluginUI.title(), m_windowId.c_str(), &visible, ImGui::GetFrameHeightWithSpacing() * 8)) {
        m_pluginUI.draw();
        switch (layoutCommonButtons(&visible)) {
        case kResponseTypeOK: {
            handleResponse(project);
            break;
        }
        case kResponseTypeCancel: {
            break;
        }
        default:
            break;
        }
    }
    close();
    return visible;
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
