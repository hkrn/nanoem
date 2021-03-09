/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "Preference.h"

#include "emapp/Error.h"
#include "emapp/FileUtils.h"
#include "emapp/StringUtils.h"
#include "emapp/ThreadedApplicationService.h"
#include "emapp/UUID.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/src/ini.h"

namespace nanoem {
namespace win32 {

const char kGlobalSectionKey[] = "nanoem.global";
const char kClientUUIDKey[] = "clientUUID";
const char kEnableAnalyticsKey[] = "enableAnalytics";
const char kEnableModelEditingKey[] = "enableModelEditing";
const char kPreferredEditingFPSKey[] = "preferredEditingFPS";
const char kRendererKey[] = "renderer";
const char kDefaultColorPixelFormatNameKey[] = "defaultColorPixelFormat";
const char kEnableSkinDeformAcceleratorKey[] = "enableSkinDeformAcceleratorGen2";
const char kHighDPIViewportModeKey[] = "highDPIViewportMode";

Preference::RandomGenerator::RandomGenerator()
{
    CryptAcquireContextW(&m_crypt, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
}

Preference::RandomGenerator::~RandomGenerator()
{
    if (m_crypt) {
        CryptReleaseContext(m_crypt, 0);
    }
}

uint32_t
Preference::RandomGenerator::gen()
{
    uint32_t value = 0;
    if (m_crypt) {
        CryptGenRandom(m_crypt, sizeof(value), reinterpret_cast<BYTE *>(&value));
    }
    return value;
}

Preference::Preference(ThreadedApplicationService *service, JSON_Value *config)
    : m_service(service)
    , m_config(config)
    , m_preference(service)
{
}

Preference::~Preference()
{
    ini_destroy(m_ini);
    m_ini = nullptr;
}

void
Preference::synchronize()
{
    m_vendorId = 0;
    m_deviceId = 0;
    m_renderer = m_preference.rendererBackend();
}

void
Preference::load()
{
    const char *iniPath =
        json_object_dotget_string(json_object(m_service->applicationConfiguration()), "win32.preference.path");
    ByteArray bytes;
    FileReaderScope scope(nullptr);
    Error error;
    if (scope.open(URI::createFromFilePath(iniPath), error)) {
        FileUtils::read(scope, bytes, error);
        bytes.push_back(0);
        m_ini = ini_load(reinterpret_cast<const char *>(bytes.data()), nullptr);
    }
    else {
        m_ini = ini_create(nullptr);
    }
    fillLoadedParameters();
    getEffectPluginPath();
}

void
Preference::save()
{
    int sectionIndex = ini_find_section(m_ini, kGlobalSectionKey, -1);
    if (sectionIndex == INI_NOT_FOUND) {
        sectionIndex = ini_section_add(m_ini, kGlobalSectionKey, -1);
    }
    const String renderer(m_preference.rendererBackend());
    set(sectionIndex, kEnableAnalyticsKey, m_preference.isAnalyticsEnabled());
    set(sectionIndex, kEnableModelEditingKey, m_preference.isModelEditingEnabled());
    set(sectionIndex, kRendererKey, renderer.c_str());
    set(sectionIndex, kPreferredEditingFPSKey, m_preferredEditingFPS);
    set(sectionIndex, kDefaultColorPixelFormatNameKey, uint32_t(m_preference.defaultColorPixelFormat()));
    set(sectionIndex, kEnableSkinDeformAcceleratorKey, m_preference.isSkinDeformAcceleratorEnabled());
    set(sectionIndex, kHighDPIViewportModeKey, uint32_t(m_preference.highDPIViewportMode()));
    if (m_preference.isResettingAnalyticsUUIDRequired()) {
        remove(sectionIndex, kClientUUIDKey);
    }
    const char *iniPath =
        json_object_dotget_string(json_object(m_service->applicationConfiguration()), "win32.preference.path");
    FileWriterScope scope;
    Error error;
    if (scope.open(URI::createFromFilePath(iniPath), false, error)) {
        int size = ini_save(m_ini, nullptr, 0);
        MutableString s(Inline::roundInt32(size));
        ini_save(m_ini, s.data(), size);
        FileUtils::write(scope.writer(), s.data(), Inline::roundInt32(size - 1), error);
        if (!error.hasReason()) {
            scope.commit(error);
        }
        else {
            scope.rollback(error);
        }
    }
}

const ApplicationPreference *
Preference::applicationPreference() const noexcept
{
    return &m_preference;
}

const char *
Preference::uuidConstString() const noexcept
{
    return m_uuid.c_str();
}

uint32_t
Preference::vendorId() const noexcept
{
    return m_vendorId;
}

uint32_t
Preference::deviceId() const noexcept
{
    return m_deviceId;
}

uint32_t
Preference::preferredEditingFPS() const noexcept
{
    return m_preferredEditingFPS;
}

void
Preference::fillLoadedParameters()
{
    int sectionIndex = ini_find_section(m_ini, kGlobalSectionKey, -1);
    if (sectionIndex == INI_NOT_FOUND) {
        sectionIndex = ini_section_add(m_ini, kGlobalSectionKey, -1);
    }
    int propertyIndex = ini_find_property(m_ini, sectionIndex, kClientUUIDKey, -1);
    if (propertyIndex != INI_NOT_FOUND) {
        const char *value = ini_property_value(m_ini, sectionIndex, propertyIndex);
        m_uuid = value;
    }
    else {
        RandomGenerator rng;
        using nanoem_uuid = nanoem::UUID;
        const nanoem_uuid &uuid = nanoem_uuid::create(rng);
        m_uuid = uuid.toString();
        ini_property_add(m_ini, sectionIndex, kClientUUIDKey, -1, m_uuid.c_str(), int(m_uuid.size()));
    }
    m_renderer = get(sectionIndex, kRendererKey, "DirectX");
    m_preferredEditingFPS = get(sectionIndex, kPreferredEditingFPSKey, 0u);
    m_preference.setAnalyticsEnabled(get(sectionIndex, kEnableAnalyticsKey, true));
    m_preference.setRendererBackend(m_renderer.c_str());
    m_preference.setModelEditingEnabled(get(sectionIndex, kEnableModelEditingKey, false));
    m_preference.setSkinDeformAcceleratorEnabled(get(sectionIndex, kEnableSkinDeformAcceleratorKey, false));
    m_preference.setHighDPIViewportMode(
        static_cast<ApplicationPreference::HighDPIViewportModeType>(get(sectionIndex, kHighDPIViewportModeKey, 0u)));
    m_preference.setDefaultColorPixelFormat(
        static_cast<sg_pixel_format>(get(sectionIndex, kDefaultColorPixelFormatNameKey, 0u)));
}

void
Preference::getEffectPluginPath()
{
    String path(json_object_dotget_string(json_object(m_config), "win32.plugin.path"));
    JSON_Object *root = json_object(m_config);
    path.append("plugin_effect.dll");
    json_object_dotset_string(root, "plugin.effect.path", path.c_str());
}

void
Preference::remove(int sectionIndex, const char *key)
{
    int propertyIndex = ini_find_property(m_ini, sectionIndex, key, -1);
    if (propertyIndex != INI_NOT_FOUND) {
        ini_property_remove(m_ini, sectionIndex, propertyIndex);
    }
}

bool
Preference::get(int sectionIndex, const char *key, bool def)
{
    int propertyIndex = ini_find_property(m_ini, sectionIndex, key, -1);
    bool value = def;
    if (propertyIndex != INI_NOT_FOUND) {
        const char *kv = ini_property_value(m_ini, sectionIndex, propertyIndex);
        value = !StringUtils::equals(kv, "false");
    }
    return value;
}

const char *
Preference::get(int sectionIndex, const char *key, const char *def)
{
    int propertyIndex = ini_find_property(m_ini, sectionIndex, key, -1);
    const char *value = def;
    if (propertyIndex != INI_NOT_FOUND) {
        value = ini_property_value(m_ini, sectionIndex, propertyIndex);
    }
    return value;
}

uint32_t
Preference::get(int sectionIndex, const char *key, uint32_t value)
{
    char buffer[16] = {};
    _itoa_s(Inline::saturateInt32(value), buffer, sizeof(buffer), 10);
    return strtoul(get(sectionIndex, key, buffer), nullptr, 10);
}

void
Preference::set(int sectionIndex, const char *key, const char *value)
{
    const int keyLength = Inline::saturateInt32(StringUtils::length(key)),
              valueLength = Inline::saturateInt32(StringUtils::length(value)),
              propertyIndex = ini_find_property(m_ini, sectionIndex, key, keyLength);
    if (propertyIndex != INI_NOT_FOUND) {
        ini_property_value_set(m_ini, sectionIndex, propertyIndex, value, valueLength);
    }
    else {
        ini_property_add(m_ini, sectionIndex, key, keyLength, value, valueLength);
    }
}

void
Preference::set(int sectionIndex, const char *key, uint32_t value)
{
    char buffer[16] = {};
    _itoa_s(Inline::saturateInt32(value), buffer, sizeof(buffer), 10);
    set(sectionIndex, key, buffer);
}

void
Preference::set(int sectionIndex, const char *key, bool value)
{
    set(sectionIndex, key, value ? "true" : "false");
}

} /* namespace win32 */
} /* namespace nanoem */
