/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_APPLICATIONPREFERENCE_H_
#define NANOEM_EMAPP_APPLICATIONPREFERENCE_H_

#include "emapp/Forward.h"

namespace nanoem {

class BaseApplicationService;

class ApplicationPreference NANOEM_DECL_SEALED : private NonCopyable {
public:
    enum HighDPIViewportModeType {
        kHighDPIViewportModeFirstEnum,
        kHighDPIViewportModeAuto = kHighDPIViewportModeFirstEnum,
        kHighDPIViewportModeEnabled,
        kHighDPIViewportModeDisabled,
        kHighDPIViewportModeMaxEnum
    };
    static const int kUndoSoftLimitDefaultValue;
    static const int kGFXBufferPoolSizeDefaultValue;
    static const int kGFXImagePoolSizeDefaultValue;
    static const int kGFXShaderPoolSizeDefaultValue;
    static const int kGFXPassPoolSizeDefaultValue;
    static const int kGFXPipelinePoolSizeDefaultValue;
    static const int kGFXUniformBufferSizeDefaultValue;

    ApplicationPreference(BaseApplicationService *application);
    ~ApplicationPreference();

    StringList allAvailableRenderers() const;
    const char *rendererBackend() const NANOEM_DECL_NOEXCEPT;
    void setRendererBackend(const char *value);
    const char *extraFontPath() const NANOEM_DECL_NOEXCEPT;
    void setExtraFontPath(const char *value);
    HighDPIViewportModeType highDPIViewportMode() const NANOEM_DECL_NOEXCEPT;
    void setHighDPIViewportMode(HighDPIViewportModeType value);
    sg_pixel_format defaultColorPixelFormat() const NANOEM_DECL_NOEXCEPT;
    void setDefaultColorPixelFormat(sg_pixel_format value);
    int preferredEditingFPS() const NANOEM_DECL_NOEXCEPT;
    void setPreferredEditingFPS(int value);
    int gfxBufferPoolSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXBufferPoolSize(int value);
    int gfxImagePoolSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXImagePoolSize(int value);
    int gfxShaderPoolSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXShaderPoolSize(int value);
    int gfxPipelinePoolSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXPipelinePoolSize(int value);
    int gfxPassPoolSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXPassPoolSize(int value);
    int gfxUniformBufferSize() const NANOEM_DECL_NOEXCEPT;
    void setGFXUniformBufferSize(int value);
    int undoSoftLimit() const NANOEM_DECL_NOEXCEPT;
    void setUndoSoftLimit(int value);
    bool isModelEditingEnabled() const NANOEM_DECL_NOEXCEPT;
    void setModelEditingEnabled(bool value);
    bool isAnalyticsEnabled() const NANOEM_DECL_NOEXCEPT;
    void setAnalyticsEnabled(bool value);
    bool isResettingAnalyticsUUIDRequired() const NANOEM_DECL_NOEXCEPT;
    void setResettingAnalyticsUUIDRequired(bool value);
    bool isSkinDeformAcceleratorEnabled() const NANOEM_DECL_NOEXCEPT;
    void setSkinDeformAcceleratorEnabled(bool value);
    bool isCrashReportEnabled() const NANOEM_DECL_NOEXCEPT;
    void setCrashReportEnabled(bool value);
    bool isEffectEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEffectEnabled(bool value);
    bool isEffectCacheEnabled() const NANOEM_DECL_NOEXCEPT;
    void setEffectCacheEnabled(bool value);

private:
    const char *readString(const char *key, const char *defaultValue) const NANOEM_DECL_NOEXCEPT;
    void writeString(const char *key, const char *value);
    int readInt(const char *key, int defaultValue) const NANOEM_DECL_NOEXCEPT;
    void writeInt(const char *key, int value);
    bool readBool(const char *key, bool defaultValue) const NANOEM_DECL_NOEXCEPT;
    void writeBool(const char *key, bool value);

    BaseApplicationService *m_application;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_APPLICATIONPREFERENCE_H_ */
