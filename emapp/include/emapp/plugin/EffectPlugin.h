/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_PLUGIN_EFFECTPLUGIN_H_
#define NANOEM_EMAPP_PLUGIN_EFFECTPLUGIN_H_

#include "emapp/plugin/BasePlugin.h"

struct nanoem_application_plugin_effect_compiler_t;

namespace nanoem {
namespace plugin {

class EffectPlugin NANOEM_DECL_SEALED : public BasePlugin {
public:
    EffectPlugin(IEventPublisher *publisher);
    ~EffectPlugin() NANOEM_DECL_NOEXCEPT;

    bool load(const URI &fileURI) NANOEM_DECL_OVERRIDE;
    void unload() NANOEM_DECL_OVERRIDE;
    bool create() NANOEM_DECL_OVERRIDE;
    void destroy() NANOEM_DECL_OVERRIDE;

    void setOption(nanoem_u32_t key, int value, Error &error);
    void setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error);
    void setOption(nanoem_u32_t key, const char *value, Error &error);
    bool compile(const URI &fileURI, ByteArray &output);
    bool compile(const String &input, ByteArray &output);
    void addIncludeSource(const String &path, const nanoem_u8_t *data, nanoem_rsize_t size);
    StringList availableExtensions() const;

    const char *failureReason() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    const char *recoverySuggestion() const NANOEM_DECL_NOEXCEPT_OVERRIDE;

private:
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetABIVersion)();
    typedef nanoem_u32_t(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetAPIVersion)();
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerInitialize)();
    typedef nanoem_application_plugin_effect_compiler_t *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerCreate)();
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetOption)(
        nanoem_application_plugin_effect_compiler_t *, nanoem_u32_t, void *, nanoem_u32_t *, int *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerSetOption)(
        nanoem_application_plugin_effect_compiler_t *, nanoem_u32_t, const void *, nanoem_u32_t, int *);
    typedef const char *const *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetAvailableExtensions)(
        nanoem_application_plugin_effect_compiler_t *, nanoem_u32_t *);
    typedef nanoem_u8_t *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerCreateBinaryFromFile)(
        nanoem_application_plugin_effect_compiler_t *, const char *, nanoem_u32_t *);
    typedef nanoem_u8_t *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory)(
        nanoem_application_plugin_effect_compiler_t *, const char *, nanoem_u32_t, nanoem_u32_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerAddIncludeSource)(
        nanoem_application_plugin_effect_compiler_t *, const char *, const nanoem_u8_t *, nanoem_u32_t);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetFailureReason)(
        const nanoem_application_plugin_effect_compiler_t *);
    typedef const char *(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerGetRecoverySuggestion)(
        const nanoem_application_plugin_effect_compiler_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerDestroyBinary)(
        nanoem_application_plugin_effect_compiler_t *, nanoem_u8_t *, nanoem_u32_t);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerDestroy)(
        nanoem_application_plugin_effect_compiler_t *);
    typedef void(APIENTRY *PFN_nanoemApplicationPluginEffectCompilerTerminate)();

    nanoem_application_plugin_effect_compiler_t *m_compiler;
    PFN_nanoemApplicationPluginEffectCompilerInitialize _effectCompilerInitialize;
    PFN_nanoemApplicationPluginEffectCompilerCreate _effectCompilerCreate;
    PFN_nanoemApplicationPluginEffectCompilerGetOption _effectCompilerGetOption;
    PFN_nanoemApplicationPluginEffectCompilerSetOption _effectCompilerSetOption;
    PFN_nanoemApplicationPluginEffectCompilerGetAvailableExtensions _effectCompilerGetAvailableExtensions;
    PFN_nanoemApplicationPluginEffectCompilerCreateBinaryFromFile _effectCompilerCreateBinaryFromFile;
    PFN_nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory _effectCompilerCreateBinaryFromMemory;
    PFN_nanoemApplicationPluginEffectCompilerAddIncludeSource _effectCompilerAddIncludeSource;
    PFN_nanoemApplicationPluginEffectCompilerGetFailureReason _effectCompilerGetFailureReason;
    PFN_nanoemApplicationPluginEffectCompilerGetRecoverySuggestion _effectCompilerGetRecoverySuggestion;
    PFN_nanoemApplicationPluginEffectCompilerDestroyBinary _effectCompilerDestroyBinary;
    PFN_nanoemApplicationPluginEffectCompilerDestroy _effectCompilerDestroy;
    PFN_nanoemApplicationPluginEffectCompilerTerminate _effectCompilerTerminate;
};

} /* namespace plugin */
} /* namespace nanoem */

#endif /* NANOEM_PLUGIN_EFFECTPLUGIN_H_ */
