/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/internal/imgui/MotionIOPluginDialog.h"

#include "emapp/PluginFactory.h"
#include "emapp/plugin/MotionIOPlugin.h"

#include "../../protoc/plugin.pb-c.h"

namespace nanoem {
namespace internal {
namespace imgui {

MotionIOPluginDialog::MotionIOPluginDialog(int functionIndex, const ByteArray &input, plugin::MotionIOPlugin *plugin,
    Project *project, Nanoem__Application__Plugin__UIWindow *window, BaseApplicationService *application)
    : BaseIOPluginDialog(functionIndex, plugin->name(), project, window, application)
    , m_input(input)
    , m_plugin(plugin)
{
}

void
MotionIOPluginDialog::handleResponse(Project *project)
{
    Error error;
    PluginFactory::MotionIOPluginProxy proxy(m_plugin);
    proxy.execute(m_functionIndex, m_input, project, error);
    error.addModalDialog(application());
}

void
MotionIOPluginDialog::handleUIComponent(const char *id, const Nanoem__Application__Plugin__UIComponent *component)
{
    PluginFactory::MotionIOPluginProxy proxy(m_plugin);
    const nanoem_rsize_t size = nanoem__application__plugin__uicomponent__get_packed_size(component);
    if (size > 0) {
        ByteArray bytes(size);
        if (nanoem__application__plugin__uicomponent__pack(component, bytes.data())) {
            proxy.setUIComponentLayout(id, bytes, m_reloadLayout, m_error);
        }
    }
}

bool
MotionIOPluginDialog::reloadUILayout(ByteArray &bytes, Error &error)
{
    PluginFactory::MotionIOPluginProxy proxy(m_plugin);
    return proxy.getUIWindowLayout(bytes, error);
}

} /* namespace imgui */
} /* namespace internal */
} /* namespace nanoem */
