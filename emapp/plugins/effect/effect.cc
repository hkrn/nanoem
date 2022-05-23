/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define NOMINMAX
#include "emapp/sdk/Effect.h"
#include "fx9/Compiler.h"

namespace {

using namespace fx9;
using namespace glslang;
using namespace nanoem::application::plugin;

struct EffectCompiler {
    static inline void
    assignOptionValue(int result, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        Inline::assignOption<int>(result, value, size, status);
    }
    static inline void
    assignOptionValue(bool result, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        Inline::assignOption<int>(result ? 1 : 0, value, size, status);
    }

    EffectCompiler()
        : m_compiler(new Compiler(ECoreProfile, EShMsgDefault))
    {
        m_compiler->setOptimizeEnabled(false);
        m_compiler->setValidationEnabled(false);
        m_compiler->setDefineMacro("NANOEM", "1");
        updateOutputShaderLanguageMacros();
    }
    ~EffectCompiler()
    {
        delete m_compiler;
        m_compiler = nullptr;
    }

    void
    getOption(nanoem_u32_t key, void *value, nanoem_u32_t *size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION: {
            assignOptionValue(m_compiler->version(), value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION: {
            assignOptionValue(m_compiler->isOptimizeEnabled(), value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION: {
            assignOptionValue(m_compiler->isValidationEnabled(), value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_ESSL: {
            assignOptionValue(m_compiler->targetLanguage() == Compiler::kLanguageTypeESSL, value, size, status);
            updateOutputShaderLanguageMacros();
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_HLSL: {
            assignOptionValue(m_compiler->targetLanguage() == Compiler::kLanguageTypeHLSL, value, size, status);
            updateOutputShaderLanguageMacros();
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_MSL: {
            assignOptionValue(m_compiler->targetLanguage() == Compiler::kLanguageTypeMSL, value, size, status);
            updateOutputShaderLanguageMacros();
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_SPIRV: {
            assignOptionValue(m_compiler->targetLanguage() == Compiler::kLanguageTypeSPIRV, value, size, status);
            updateOutputShaderLanguageMacros();
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_ENABLE_MME_MIPMAP: {
            assignOptionValue(m_compiler->containsDefineMacro("MME_MIPMAP"), value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_u32_t size, nanoem_application_plugin_status_t *status)
    {
        bool unknown = false;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_compiler->setVersion(*static_cast<const int *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION: {
            if (Inline::validateArgument<int>(value, size, status)) {
                bool enabled = *static_cast<const int *>(value) != 0;
                m_compiler->setOptimizeEnabled(enabled);
                m_compiler->setDefineMacro("NANOEM_OUTPUT_OPTIMIZED", enabled ? "1" : "0");
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION: {
            if (Inline::validateArgument<int>(value, size, status)) {
                m_compiler->setValidationEnabled(*static_cast<const int *>(value) != 0);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_ESSL: {
            if (Inline::validateArgument<int>(value, size, status) && *static_cast<const int *>(value) != 0) {
                m_compiler->setTargetLanguage(Compiler::kLanguageTypeESSL);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_HLSL: {
            if (Inline::validateArgument<int>(value, size, status) && *static_cast<const int *>(value) != 0) {
                m_compiler->setTargetLanguage(Compiler::kLanguageTypeHLSL);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_MSL: {
            if (Inline::validateArgument<int>(value, size, status) && *static_cast<const int *>(value) != 0) {
                m_compiler->setTargetLanguage(Compiler::kLanguageTypeMSL);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_SPIRV: {
            if (Inline::validateArgument<int>(value, size, status) && *static_cast<const int *>(value) != 0) {
                m_compiler->setTargetLanguage(Compiler::kLanguageTypeSPIRV);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_ENABLE_MME_MIPMAP: {
            if (Inline::validateArgument<int>(value, size, status)) {
                if (*static_cast<const int *>(value) != 0) {
                    m_compiler->setDefineMacro("MME_MIPMAP", "1");
                }
                else {
                    m_compiler->removeDefineMacro("MME_MIPMAP");
                }
            }
            break;
        }
        default:
            unknown = true;
            break;
        }
        nanoem_application_plugin_status_assign_error(status,
            unknown ? NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION : NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS);
    }
    nanoem_u8_t *
    compile(const char *path, nanoem_u32_t *size)
    {
        Compiler::EffectProduct effectProduct;
        nanoem_u8_t *data = handleCompile(m_compiler->compile(path, effectProduct), effectProduct, size);
        return data;
    }
    nanoem_u8_t *
    compile(const char *source, nanoem_u32_t length, nanoem_u32_t *output_size)
    {
        Compiler::EffectProduct effectProduct;
        const std::string sourceString(source, length);
        nanoem_u8_t *data =
            handleCompile(m_compiler->compile(sourceString, "", effectProduct), effectProduct, output_size);
        return data;
    }
    void
    addIncludeSource(const char *path, const nanoem_u8_t *source, nanoem_u32_t length)
    {
        m_compiler->addIncludeSource(std::string(path), std::string(reinterpret_cast<const char *>(source), length));
    }
    void
    destroy(nanoem_u8_t *data)
    {
        delete[] data;
    }

    const char *
    failureReason() const
    {
        return m_reason.c_str();
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }

    nanoem_u8_t *
    handleCompile(bool result, Compiler::EffectProduct &effectProduct, nanoem_u32_t *size)
    {
        const Compiler::EffectProduct::LogSink &sink = effectProduct.sink;
        nanoem_u8_t *data = nullptr;
        if (result) {
            auto message = effectProduct.message;
            size_t s = message.size();
            data = new nanoem_u8_t[s];
            *size = Inline::saturateInt32U(s);
            memcpy(data, message.data(), s);
        }
        else if (!sink.validator.empty()) {
            m_reason = sink.validator;
        }
        else if (!sink.translator.empty()) {
            std::string reason;
            for (const auto &message : sink.translator) {
                reason.append(message);
                reason.append("\n");
            }
            m_reason = reason;
        }
        else {
            m_reason = sink.info;
        }
        return data;
    }
    void
    updateOutputShaderLanguageMacros()
    {
        const Compiler::LanguageType language = m_compiler->targetLanguage();
        m_compiler->setDefineMacro(
            "NANOEM_OUTPUT_SHADER_LANGUAGE_GLSL", language == Compiler::kLanguageTypeGLSL ? "1" : "0");
        m_compiler->setDefineMacro(
            "NANOEM_OUTPUT_SHADER_LANGUAGE_ESSL", language == Compiler::kLanguageTypeESSL ? "1" : "0");
        m_compiler->setDefineMacro(
            "NANOEM_OUTPUT_SHADER_LANGUAGE_HLSL", language == Compiler::kLanguageTypeHLSL ? "1" : "0");
        m_compiler->setDefineMacro(
            "NANOEM_OUTPUT_SHADER_LANGUAGE_MSL", language == Compiler::kLanguageTypeMSL ? "1" : "0");
        m_compiler->setDefineMacro(
            "NANOEM_OUTPUT_SHADER_LANGUAGE_SPIRV", language == Compiler::kLanguageTypeSPIRV ? "1" : "0");
    }

    Compiler *m_compiler;
    std::string m_reason;
};

} /* namespace anonymous */

struct nanoem_application_plugin_effect_compiler_t : EffectCompiler { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginEffectCompilerGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerInitialize(void)
{
    Compiler::initialize();
}

nanoem_application_plugin_effect_compiler_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreate()
{
    return new nanoem_application_plugin_effect_compiler_t();
}

void APIENTRY
nanoemApplicationPluginEffectCompilerGetOption(nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key,
    void *value, nanoem_u32_t *size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(plugin)) {
        plugin->getOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEffectCompilerSetOption(nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key,
    const void *value, nanoem_u32_t size, nanoem_i32_t *status)
{
    nanoem_application_plugin_status_t *statusPtr = reinterpret_cast<nanoem_application_plugin_status_t *>(status);
    if (nanoem_is_not_null(plugin)) {
        plugin->setOption(key, value, size, statusPtr);
    }
    else {
        nanoem_application_plugin_status_assign_error(statusPtr, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginEffectCompilerGetAvailableExtensions(
    nanoem_application_plugin_effect_compiler_t * /* plugin */, nanoem_u32_t *num_extensions)
{
    static const char *kExtensions[] = { "fx", "fxsub" };
    *num_extensions = sizeof(kExtensions) / sizeof(kExtensions[0]);
    return kExtensions;
}

nanoem_u8_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreateBinaryFromFile(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, nanoem_u32_t *size)
{
    return nanoem_is_not_null(plugin) ? plugin->compile(path, size) : nullptr;
}

nanoem_u8_t *APIENTRY
nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *source, nanoem_u32_t length, nanoem_u32_t *size)
{
    return nanoem_is_not_null(plugin) ? plugin->compile(source, length, size) : nullptr;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerAddIncludeSource(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, const nanoem_u8_t *data, nanoem_u32_t size)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->addIncludeSource(path, data, size);
    }
}

const char *APIENTRY
nanoemApplicationPluginEffectCompilerGetFailureReason(const nanoem_application_plugin_effect_compiler_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginEffectCompilerGetRecoverySuggestion(const nanoem_application_plugin_effect_compiler_t *plugin)
{
    return nanoem_is_not_null(plugin) ? plugin->recoverySuggestion() : nullptr;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerDestroyBinary(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u8_t *data, nanoem_u32_t /* size */)
{
    if (nanoem_is_not_null(plugin)) {
        plugin->destroy(data);
    }
}

void APIENTRY
nanoemApplicationPluginEffectCompilerDestroy(nanoem_application_plugin_effect_compiler_t *plugin)
{
    delete plugin;
}

void APIENTRY
nanoemApplicationPluginEffectCompilerTerminate(void)
{
    Compiler::terminate();
}
