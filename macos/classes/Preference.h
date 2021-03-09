/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#import <Foundation/Foundation.h>

#include "emapp/ApplicationPreference.h"

#include "nanoem/ext/parson/parson.h"

namespace nanoem {

class ThreadedApplicationService;

namespace macos {

class Preference : public nanoem::NonCopyable {
public:
    static bool isMetalAvailable();

    Preference(NSUserDefaults *defaults, nanoem::ThreadedApplicationService *service, JSON_Value *config);
    ~Preference();

    const ApplicationPreference *applicationPreference() const;
    ApplicationPreference *mutableApplicationPreference();
    NSString *clientUUID() const;
    uint32_t vendorId() const;
    uint32_t deviceId() const;

private:
    void load();

    ApplicationPreference m_preference;
    NSUserDefaults *m_defaults = nil;
    NSString *m_clientUUID = nil;
    JSON_Value *m_config = nullptr;
    uint32_t m_vendorId = 0;
    uint32_t m_deviceId = 0;
};

} /* namespace macos */
} /* namespace nanoem */
