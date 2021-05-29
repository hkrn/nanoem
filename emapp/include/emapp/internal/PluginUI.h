/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_INTERNAL_PLUGINUI_H_
#define NANOEM_EMAPP_INTERNAL_PLUGINUI_H_

#include "emapp/Forward.h"
#include "imgui/imgui.h"

struct Nanoem__Application__Plugin__UIComponent;
struct Nanoem__Application__Plugin__UIWindow;

namespace nanoem {

class Error;

namespace internal {

class PluginUI : private NonCopyable {
public:
    class IReactor {
    public:
        virtual void handleUIComponent(const char *id, const Nanoem__Application__Plugin__UIComponent *component) = 0;
        virtual bool reloadUILayout(ByteArray &bytes, Error &error) = 0;
    };

    PluginUI(
        IReactor *reactor, Nanoem__Application__Plugin__UIWindow *window, const char *title, float devicePixelRatio);
    ~PluginUI();

    void draw();
    bool reload(Error &error);
    void clear();

    const char *title() const NANOEM_DECL_NOEXCEPT;

private:
    void drawComponent(const Nanoem__Application__Plugin__UIComponent *component);

    IReactor *m_reactor;
    Nanoem__Application__Plugin__UIWindow *m_window;
    const char *m_title;
    const float m_devicePixelRatio;
};

} /* namespace internal */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_INTERNAL_PLUGINUI_H_ */
