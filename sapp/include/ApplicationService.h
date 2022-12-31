/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_SAPP_APPLICAIONSERVICE_H_
#define NANOEM_EMAPP_SAPP_APPLICAIONSERVICE_H_

#include "emapp/BaseApplicationService.h"

#include "ApplicationClient.h"

namespace nanoem {
namespace sapp {

class ApplicationService final : public BaseApplicationService {
public:
    ApplicationService(const JSON_Value *config, ApplicationClient::Bridge *bridge);
    ~ApplicationService() noexcept;

    void draw();
    void consumeDispatchAllCommandMessages();
    void dispatchAllEventMessages();

private:
    BaseApplicationClient *menubarApplicationClient() override;
    Project::ISkinDeformerFactory *createSkinDeformerFactory() override;
    IAudioPlayer *createAudioPlayer() override;
    bool isRendererAvailable(const char *value) const noexcept override;
    void handleSetupGraphicsEngine(sg_desc &desc) override;
    void sendEventMessage(const Nanoem__Application__Event *event) override;

    ApplicationClient m_menubarApplciationClient;
    ApplicationClient::Bridge *m_bridge;
    void *m_dllHandle = nullptr;
};

} /* namespace sapp */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_SAPP_APPLICAIONSERVICE_H_ */
