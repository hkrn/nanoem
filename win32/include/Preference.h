/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_PREFERENCE_H_
#define NANOEM_EMAPP_WIN32_PREFERENCE_H_

#include "emapp/Forward.h"

#include "emapp/ApplicationPreference.h"
#include "nanoem/ext/parson/parson.h"

#include <wincrypt.h>

struct ini_t;

namespace nanoem {

class ThreadedApplicationService;

namespace win32 {

class Preference final {
public:
    Preference(ThreadedApplicationService *service, JSON_Value *config);
    ~Preference();

    void synchronize();
    void load();
    void save();

    const ApplicationPreference *applicationPreference() const noexcept;
    const char *uuidConstString() const noexcept;
    uint32_t vendorId() const noexcept;
    uint32_t deviceId() const noexcept;
    uint32_t preferredEditingFPS() const noexcept;

private:
    struct RandomGenerator {
        RandomGenerator();
        ~RandomGenerator();
        uint32_t gen();
        HCRYPTPROV m_crypt = 0;
    };

    void fillLoadedParameters();
    void getEffectPluginPath();
    void remove(int sectionIndex, const char *key);
    bool get(int sectionIndex, const char *key, bool def);
    const char *get(int sectionIndex, const char *key, const char *def);
    uint32_t get(int sectionIndex, const char *key, uint32_t value);
    void set(int sectionIndex, const char *key, const char *value);
    void set(int sectionIndex, const char *key, uint32_t value);
    void set(int sectionIndex, const char *key, bool value);

    ThreadedApplicationService *m_service;
    JSON_Value *m_config;
    ApplicationPreference m_preference;
    String m_renderer;
    uint32_t m_vendorId = 0;
    uint32_t m_deviceId = 0;
    uint32_t m_preferredEditingFPS = 0;
    String m_uuid;
    ini_t *m_ini = nullptr;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_PREFERENCE_H_ */
