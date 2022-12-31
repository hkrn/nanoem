/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/ApplicationPreference.h"

#include "emapp/BaseApplicationService.h"
#include "emapp/StringUtils.h"

#include "undo/undo.h"

namespace nanoem {
namespace {

static const char kPreferenceKeyPrefix[] = "application.preference";
static const char kRendererBackend[] = "renderer.backend";
static const char kFontPath[] = "font.path";
static const char kDefaultColorPixelFormat[] = "renderer.colorPixelFormat";
static const char kModelEditingEnabled[] = "editing.model.enabled";
static const char kAnalyticsEnabled[] = "analytics.enabled";
static const char kResettingAnalyticsUUIDRequired[] = "analytics.reset";
static const char kPreferredEditingFPS[] = "editing.motion.fps";
static const char kSkinDeformAcceleratorEnabled[] = "renderer.sda.enabled";
static const char kCrashReporterEnabled[] = "crashReporter.enabled";
static const char kUndoSoftLimit[] = "undo.limit";
static const char kEffectEnabled[] = "effect.enabled";
static const char kEffectCacheEnabled[] = "effect.cached";
static const char kHighDPIViewportMode[] = "viewport.highDPI";
static const char kGFXBufferPoolSize[] = "gfx.pool.buffer";
static const char kGFXImagePoolSize[] = "gfx.pool.image";
static const char kGFXShaderPoolSize[] = "gfx.pool.shader";
static const char kGFXPassPoolSize[] = "gfx.pool.pass";
static const char kGFXPipelinePoolSize[] = "gfx.pool.pipeline";
static const char kGFXUniformBufferSize[] = "gfx.buffer.uniform";
static const int kBufferSize = 128;

} /* namespace anonymous */

const int ApplicationPreference::kUndoSoftLimitDefaultValue = 64;
const int ApplicationPreference::kGFXBufferPoolSizeDefaultValue = 0x2000;
const int ApplicationPreference::kGFXImagePoolSizeDefaultValue = 0x8000;
const int ApplicationPreference::kGFXShaderPoolSizeDefaultValue = 0x2000;
const int ApplicationPreference::kGFXPassPoolSizeDefaultValue = 0x2000;
const int ApplicationPreference::kGFXPipelinePoolSizeDefaultValue = 0x4000;
const int ApplicationPreference::kGFXUniformBufferSizeDefaultValue = 0x800000;

ApplicationPreference::ApplicationPreference(BaseApplicationService *application)
    : m_application(application)
{
}

ApplicationPreference::~ApplicationPreference()
{
}

StringList
ApplicationPreference::allAvailableRenderers() const
{
    const char *renderers[] = { BaseApplicationService::kRendererOpenGL, BaseApplicationService::kRendererDirectX,
        BaseApplicationService::kRendererMetal };
    StringList values;
    for (nanoem_rsize_t i = 0; i < BX_COUNTOF(renderers); i++) {
        if (m_application->isRendererAvailable(renderers[i])) {
            values.push_back(renderers[i]);
        }
    }
    return values;
}

const char *
ApplicationPreference::rendererBackend() const NANOEM_DECL_NOEXCEPT
{
#if defined(_WIN32)
    return readString(kRendererBackend, BaseApplicationService::kRendererDirectX);
#elif defined(__APPLE__)
    return readString(kRendererBackend, BaseApplicationService::kRendererMetal);
#else
    return readString(kRendererBackend, BaseApplicationService::kRendererOpenGL);
#endif
}

void
ApplicationPreference::setRendererBackend(const char *value)
{
    writeString(kRendererBackend, value);
}

const char *
ApplicationPreference::extraFontPath() const NANOEM_DECL_NOEXCEPT
{
    return readString(kFontPath, nullptr);
}

void
ApplicationPreference::setExtraFontPath(const char *value)
{
    writeString(kFontPath, value);
}

ApplicationPreference::HighDPIViewportModeType
ApplicationPreference::highDPIViewportMode() const NANOEM_DECL_NOEXCEPT
{
    const int rawValue = readInt(kHighDPIViewportMode, kHighDPIViewportModeAuto);
    const HighDPIViewportModeType value = static_cast<HighDPIViewportModeType>(rawValue);
    switch (value) {
    case kHighDPIViewportModeAuto:
    case kHighDPIViewportModeEnabled:
    case kHighDPIViewportModeDisabled:
        return value;
    default:
        return kHighDPIViewportModeAuto;
    }
}

void
ApplicationPreference::setHighDPIViewportMode(HighDPIViewportModeType value)
{
    writeInt(kHighDPIViewportMode, value);
}

sg_pixel_format
ApplicationPreference::defaultColorPixelFormat() const NANOEM_DECL_NOEXCEPT
{
    const int rawValue = readInt(kHighDPIViewportMode, SG_PIXELFORMAT_RGBA8);
    const sg_pixel_format pixelFormat = static_cast<sg_pixel_format>(rawValue);
    switch (pixelFormat) {
    case SG_PIXELFORMAT_RGBA8:
    case SG_PIXELFORMAT_RGB10A2:
    case SG_PIXELFORMAT_RGBA16F: {
        return pixelFormat;
    }
    default:
        return SG_PIXELFORMAT_RGBA8;
    }
}

void
ApplicationPreference::setDefaultColorPixelFormat(sg_pixel_format value)
{
    writeInt(kDefaultColorPixelFormat, static_cast<int>(value));
}

int
ApplicationPreference::preferredEditingFPS() const NANOEM_DECL_NOEXCEPT
{
    return readInt(kPreferredEditingFPS, 60);
}

void
ApplicationPreference::setPreferredEditingFPS(int value)
{
    writeInt(kPreferredEditingFPS, value);
}

int
ApplicationPreference::gfxBufferPoolSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXBufferPoolSize, kGFXBufferPoolSizeDefaultValue), 1024, 0xffff);
}

void
ApplicationPreference::setGFXBufferPoolSize(int value)
{
    writeInt(kGFXBufferPoolSize, value);
}

int
ApplicationPreference::gfxImagePoolSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXImagePoolSize, kGFXImagePoolSizeDefaultValue), 4096, 0xffff);
}
void
ApplicationPreference::setGFXImagePoolSize(int value)
{
    writeInt(kGFXImagePoolSize, value);
}

int
ApplicationPreference::gfxShaderPoolSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXShaderPoolSize, kGFXShaderPoolSizeDefaultValue), 1024, 0xffff);
}

void
ApplicationPreference::setGFXShaderPoolSize(int value)
{
    writeInt(kGFXShaderPoolSize, value);
}

int
ApplicationPreference::gfxPipelinePoolSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXPipelinePoolSize, kGFXPipelinePoolSizeDefaultValue), 1024, 0xffff);
}

void
ApplicationPreference::setGFXPipelinePoolSize(int value)
{
    writeInt(kGFXPipelinePoolSize, value);
}

int
ApplicationPreference::gfxPassPoolSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXPassPoolSize, kGFXPassPoolSizeDefaultValue), 512, 0xffff);
}

void
ApplicationPreference::setGFXPassPoolSize(int value)
{
    writeInt(kGFXPassPoolSize, value);
}

int
ApplicationPreference::gfxUniformBufferSize() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(readInt(kGFXUniformBufferSize, kGFXUniformBufferSizeDefaultValue), 0x10000, 0x7fffff);
}

void
ApplicationPreference::setGFXUniformBufferSize(int value)
{
    writeInt(kGFXUniformBufferSize, value);
}

int
ApplicationPreference::undoSoftLimit() const NANOEM_DECL_NOEXCEPT
{
    return glm::clamp(
        readInt(kUndoSoftLimit, kUndoSoftLimitDefaultValue), kUndoSoftLimitDefaultValue, undoStackGetHardLimit());
}

void
ApplicationPreference::setUndoSoftLimit(int value)
{
    writeInt(kUndoSoftLimit, value);
}

bool
ApplicationPreference::isModelEditingEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kModelEditingEnabled, false);
}

void
ApplicationPreference::setModelEditingEnabled(bool value)
{
    writeBool(kModelEditingEnabled, value);
}

bool
ApplicationPreference::isAnalyticsEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kAnalyticsEnabled, true);
}

void
ApplicationPreference::setAnalyticsEnabled(bool value)
{
    writeBool(kAnalyticsEnabled, value);
}

bool
ApplicationPreference::isResettingAnalyticsUUIDRequired() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kResettingAnalyticsUUIDRequired, false);
}

void
ApplicationPreference::setResettingAnalyticsUUIDRequired(bool value)
{
    writeBool(kResettingAnalyticsUUIDRequired, value);
}

bool
ApplicationPreference::isSkinDeformAcceleratorEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kSkinDeformAcceleratorEnabled, false);
}

void
ApplicationPreference::setSkinDeformAcceleratorEnabled(bool value)
{
    writeBool(kSkinDeformAcceleratorEnabled, value);
}

bool
ApplicationPreference::isCrashReportEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kCrashReporterEnabled, true);
}

void
ApplicationPreference::setCrashReportEnabled(bool value)
{
    writeBool(kCrashReporterEnabled, value);
}

bool
ApplicationPreference::isEffectEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kEffectEnabled, false);
}

void
ApplicationPreference::setEffectEnabled(bool value)
{
    writeBool(kEffectEnabled, value);
}

bool
ApplicationPreference::isEffectCacheEnabled() const NANOEM_DECL_NOEXCEPT
{
    return readBool(kEffectCacheEnabled, false);
}

void
ApplicationPreference::setEffectCacheEnabled(bool value)
{
    writeBool(kEffectCacheEnabled, value);
}

const char *
ApplicationPreference::readString(const char *key, const char *defaultValue) const NANOEM_DECL_NOEXCEPT
{
    const JSON_Object *config = json_object(m_application->applicationConfiguration());
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    const char *result;
    if (const JSON_Value *value = json_object_dotget_value(pending, key)) {
        result = json_value_get_string(value);
    }
    else if (const JSON_Value *value = json_object_dotget_value(config, key)) {
        result = json_value_get_string(value);
    }
    else {
        result = defaultValue;
    }
    return result;
}

void
ApplicationPreference::writeString(const char *key, const char *value)
{
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    json_object_dotset_string(pending, key, value);
}

int
ApplicationPreference::readInt(const char *key, int defaultValue) const NANOEM_DECL_NOEXCEPT
{
    const JSON_Object *config = json_object(m_application->applicationConfiguration());
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    int result;
    char keyBuffer[kBufferSize];
    StringUtils::format(keyBuffer, sizeof(keyBuffer), "%s.%s", kPreferenceKeyPrefix, key);
    if (const JSON_Value *value = json_object_dotget_value(pending, keyBuffer)) {
        result = static_cast<int>(json_value_get_number(value));
    }
    else if (const JSON_Value *value = json_object_dotget_value(config, keyBuffer)) {
        result = static_cast<int>(json_value_get_number(value));
    }
    else {
        result = defaultValue;
    }
    return result;
}

void
ApplicationPreference::writeInt(const char *key, int value)
{
    char keyBuffer[kBufferSize];
    StringUtils::format(keyBuffer, sizeof(keyBuffer), "%s.%s", kPreferenceKeyPrefix, key);
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    json_object_dotset_number(pending, keyBuffer, value);
}

bool
ApplicationPreference::readBool(const char *key, bool defaultValue) const NANOEM_DECL_NOEXCEPT
{
    const JSON_Object *config = json_object(m_application->applicationConfiguration());
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    bool result;
    char keyBuffer[kBufferSize];
    StringUtils::format(keyBuffer, sizeof(keyBuffer), "%s.%s", kPreferenceKeyPrefix, key);
    if (const JSON_Value *value = json_object_dotget_value(pending, keyBuffer)) {
        result = json_value_get_boolean(value);
    }
    else if (const JSON_Value *value = json_object_dotget_value(config, keyBuffer)) {
        result = json_value_get_boolean(value);
    }
    else {
        result = defaultValue;
    }
    return result;
}

void
ApplicationPreference::writeBool(const char *key, bool value)
{
    char keyBuffer[kBufferSize];
    StringUtils::format(keyBuffer, sizeof(keyBuffer), "%s.%s", kPreferenceKeyPrefix, key);
    JSON_Object *pending = json_object(m_application->applicationPendingChangeConfiguration());
    json_object_dotset_boolean(pending, keyBuffer, value);
}

} /* namespace nanoem */
