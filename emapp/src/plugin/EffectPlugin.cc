/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/plugin/EffectPlugin.h"

#include "emapp/Error.h"
#include "emapp/IEventPublisher.h"
#include "emapp/StringUtils.h"
#include "emapp/URI.h"
#include "emapp/private/CommonInclude.h"
#include "emapp/sdk/Effect.h"

namespace nanoem {
namespace plugin {

EffectPlugin::EffectPlugin(IEventPublisher *publisher)
    : BasePlugin(publisher)
    , m_compiler(nullptr)
    , _effectCompilerInitialize(nullptr)
    , _effectCompilerCreate(nullptr)
    , _effectCompilerGetOption(nullptr)
    , _effectCompilerSetOption(nullptr)
    , _effectCompilerGetAvailableExtensions(nullptr)
    , _effectCompilerCreateBinaryFromFile(nullptr)
    , _effectCompilerCreateBinaryFromMemory(nullptr)
    , _effectCompilerAddIncludeSource(nullptr)
    , _effectCompilerGetFailureReason(nullptr)
    , _effectCompilerGetRecoverySuggestion(nullptr)
    , _effectCompilerDestroyBinary(nullptr)
    , _effectCompilerDestroy(nullptr)
    , _effectCompilerTerminate(nullptr)
{
}

EffectPlugin::~EffectPlugin() NANOEM_DECL_NOEXCEPT
{
    unload();
}

bool
EffectPlugin::load(const URI &fileURI)
{
#if defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    BX_UNUSED_1(fileURI);
    bool succeeded = true;
    _effectCompilerInitialize = nanoemApplicationPluginEffectCompilerInitialize;
    _effectCompilerCreate = nanoemApplicationPluginEffectCompilerCreate;
    _effectCompilerGetOption = nanoemApplicationPluginEffectCompilerGetOption;
    _effectCompilerSetOption = nanoemApplicationPluginEffectCompilerSetOption;
    _effectCompilerGetAvailableExtensions = nanoemApplicationPluginEffectCompilerGetAvailableExtensions;
    _effectCompilerCreateBinaryFromFile = nanoemApplicationPluginEffectCompilerCreateBinaryFromFile;
    _effectCompilerCreateBinaryFromMemory = nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory;
    _effectCompilerAddIncludeSource = nanoemApplicationPluginEffectCompilerAddIncludeSource;
    _effectCompilerGetFailureReason = nanoemApplicationPluginEffectCompilerGetFailureReason;
    _effectCompilerGetRecoverySuggestion = nanoemApplicationPluginEffectCompilerGetRecoverySuggestion;
    _effectCompilerDestroyBinary = nanoemApplicationPluginEffectCompilerDestroyBinary;
    _effectCompilerDestroy = nanoemApplicationPluginEffectCompilerDestroy;
    _effectCompilerTerminate = nanoemApplicationPluginEffectCompilerTerminate;
    _effectCompilerInitialize();
#else /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    bool succeeded = m_handle != nullptr;
    if (!succeeded) {
        bool valid = true;
        if (void *handle = bx::dlopen(fileURI.absolutePathConstString())) {
            PFN_nanoemApplicationPluginEffectCompilerGetABIVersion _effectCompilerGetABIVersion = nullptr;
            EMLOG_DEBUG("Loading effect plugin path={} handle={}", fileURI.absolutePathConstString(), handle);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerGetABIVersion", _effectCompilerGetABIVersion, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerInitialize", _effectCompilerInitialize, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerCreate", _effectCompilerCreate, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerGetOption", _effectCompilerGetOption, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerSetOption", _effectCompilerSetOption, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerGetAvailableExtensions",
                _effectCompilerGetAvailableExtensions, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerCreateBinaryFromFile",
                _effectCompilerCreateBinaryFromFile, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory",
                _effectCompilerCreateBinaryFromMemory, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerAddIncludeSource",
                _effectCompilerAddIncludeSource, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerGetFailureReason",
                _effectCompilerGetFailureReason, valid);
            Inline::resolveSymbol(handle, "nanoemApplicationPluginEffectCompilerGetRecoverySuggestion",
                _effectCompilerGetRecoverySuggestion, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerDestroyBinary", _effectCompilerDestroyBinary, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerDestroy", _effectCompilerDestroy, valid);
            Inline::resolveSymbol(
                handle, "nanoemApplicationPluginEffectCompilerTerminate", _effectCompilerTerminate, valid);
            if (valid &&
                isABICompatible(
                    _effectCompilerGetABIVersion(), NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MAJOR)) {
                m_handle = handle;
                m_name = fileURI.lastPathComponent();
                _effectCompilerInitialize();
                succeeded = true;
            }
            else {
                _effectCompilerTerminate = nullptr;
                bx::dlclose(handle);
            }
        }
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
    return succeeded;
}

void
EffectPlugin::unload()
{
    if (_effectCompilerTerminate) {
        _effectCompilerTerminate();
        _effectCompilerTerminate = nullptr;
    }
#if !defined(NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN)
    if (m_handle) {
        bx::dlclose(m_handle);
        m_handle = nullptr;
    }
#endif /* NANOEM_ENABLE_STATIC_BUNDLE_PLUGIN */
}

bool
EffectPlugin::create()
{
    if (!m_compiler) {
        m_compiler = _effectCompilerCreate();
    }
    return m_compiler != nullptr;
}

void
EffectPlugin::destroy()
{
    if (_effectCompilerDestroy) {
        _effectCompilerDestroy(m_compiler);
        m_compiler = nullptr;
    }
}

void
EffectPlugin::setOption(nanoem_u32_t key, int value, Error &error)
{
    const int v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _effectCompilerSetOption(m_compiler, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
EffectPlugin::setOption(nanoem_u32_t key, nanoem_u32_t value, Error &error)
{
    const nanoem_u32_t v = value;
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _effectCompilerSetOption(m_compiler, key, &v, Inline::saturateInt32U(sizeof(v)), &status);
    handlePluginStatus(status, error);
}

void
EffectPlugin::setOption(nanoem_u32_t key, const char *value, Error &error)
{
    int status = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
    _effectCompilerSetOption(m_compiler, key, value, value ? StringUtils::length(value) : 0, &status);
    handlePluginStatus(status, error);
}

bool
EffectPlugin::compile(const URI &fileURI, ByteArray &output)
{
    nanoem_u32_t size;
    output.clear();
    if (nanoem_u8_t *data = _effectCompilerCreateBinaryFromFile(m_compiler, fileURI.absolutePath().c_str(), &size)) {
        output.assign(data, data + size);
        _effectCompilerDestroyBinary(m_compiler, data, size);
    }
    return !output.empty();
}

bool
EffectPlugin::compile(const String &input, ByteArray &output)
{
    nanoem_u32_t size;
    output.clear();
    if (nanoem_u8_t *data = _effectCompilerCreateBinaryFromMemory(m_compiler, input.c_str(), input.size(), &size)) {
        output.assign(data, data + size);
        _effectCompilerDestroyBinary(m_compiler, data, size);
    }
    return !output.empty();
}

void
EffectPlugin::addIncludeSource(const String &path, const nanoem_u8_t *data, nanoem_rsize_t size)
{
    if (_effectCompilerAddIncludeSource) {
        _effectCompilerAddIncludeSource(m_compiler, path.c_str(), data, Inline::saturateInt32U(size));
    }
}

StringList
EffectPlugin::availableExtensions() const
{
    nanoem_u32_t length;
    StringList extensionList;
    if (const char *const *allExtensions = _effectCompilerGetAvailableExtensions(m_compiler, &length)) {
        extensionList.clear();
        for (nanoem_u32_t i = 0; i < length; i++) {
            extensionList.push_back(allExtensions[i]);
        }
    }
    return extensionList;
}

const char *
EffectPlugin::failureReason() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _effectCompilerGetFailureReason(m_compiler);
    return ptr ? ptr : "";
}

const char *
EffectPlugin::recoverySuggestion() const NANOEM_DECL_NOEXCEPT
{
    const char *ptr = _effectCompilerGetRecoverySuggestion(m_compiler);
    return ptr ? ptr : "";
}

} /* namespace plugin */
} /* namespace nanoem */
