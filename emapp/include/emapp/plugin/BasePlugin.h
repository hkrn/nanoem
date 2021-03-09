/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_BASEPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_BASEPLUGIN_H_

#include "emapp/Forward.h"

namespace nanoem {

class Error;
class IAudioPlayer;
class ICamera;
class IEventPublisher;
class ILight;
class URI;

namespace plugin {

class BasePlugin : private NonCopyable {
public:
    BasePlugin(IEventPublisher *publisher);
    virtual ~BasePlugin() NANOEM_DECL_NOEXCEPT;

    virtual bool load(const URI &fileURI) = 0;
    virtual void unload() = 0;
    virtual bool create() = 0;
    virtual void destroy() = 0;

    virtual const char *failureReason() const NANOEM_DECL_NOEXCEPT = 0;
    virtual const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT = 0;

protected:
    static void encodeAudioDescription(const IAudioPlayer *player, ByteArray &bytes);
    static void encodeCameraDescription(const ICamera *camera, ByteArray &bytes);
    static void encodeLightDescription(const ILight *light, ByteArray &bytes);
    static void encodeVector4(const Vector3 &value, void *dest);
    static bool isABICompatible(nanoem_u32_t version, nanoem_u32_t major) NANOEM_DECL_NOEXCEPT;
    void handlePluginStatus(int result, Error &error);

    IEventPublisher *m_eventPublisher;
    String m_name;
    void *m_handle;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_PLUGIN_BASEPLUGIN_H_ */
