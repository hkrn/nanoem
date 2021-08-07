/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_EFFECT_H_
#define EMAPP_PLUGIN_SDK_EFFECT_H_

#include "Common.h"

/**
 * \defgroup emapp nanoem Application (emapp)
 * @{
 */

/**
 * \defgroup emapp_plugin_effect nanoem Application Effect Compiler Plugin
 * @{
 */

#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION                                                          \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MAJOR,            \
        NANOEM_APPLICATION_PLUGIN_EFFECT_COMPILER_ABI_VERSION_MINOR)

NANOEM_DECL_ENUM(int, nanoem_application_plugin_effect_option_t) {
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_UNKNOWN = -1, ///< Unknown
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_SHADER_VERSION = NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_FIRST_ENUM, ///< Setting shader version (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OPTIMIZATION, ///< Setting shader optimization enabled (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_VALIDATION, ///< Setting shader validation enabled (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_ESSL, ///< Setting shader output ESSL (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_HLSL, ///< Setting shader output HLSL (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_MSL, ///< Setting shader output MSL (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_OUTPUT_SPIRV, ///< Setting shader output SPIRV (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_ENABLE_MME_MIPMAP, ///< Setting MME mipmap macro enabled (u32)
    NANOEM_APPLICATION_PLUGIN_EFFECT_OPTION_MAX_ENUM };

/**
 * \brief The opaque effect compiler plugin object
 */
typedef struct nanoem_application_plugin_effect_compiler_t nanoem_application_plugin_effect_compiler_t;

/**
 * \brief Get plugin's ABI version
 *
 * Plugin ABI version consists major version and minor version.
 *
 * If the major version differs from nanoem runtime version, The plugin will not be loaded.
 * If the minor version is greater than nanoem runtime version, corresponding functions will not be called.
 *
 * \return The plugin ABI version made with \b NANOEM_APPLICATION_PLUGIN_MODEL_ABI_VERSION
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginEffectCompilerGetABIVersion(void);

/**
 * \brief Initialize the plugin
 *
 * The function will call once at the plugin initialization.
 *
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerInitialize(void);

/**
 * \brief Create an opaque effect compiler plugin object
 *
 * \return The opaque effect compiler plugin object
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_application_plugin_effect_compiler_t *APIENTRY nanoemApplicationPluginEffectCompilerCreate();

/**
 * \brief Get the effect compiler option value
 *
 * \param plugin The opaque effect compiler plugin object
 * \param key key of \b nanoem_application_plugin_effect_option_t
 * \param[out] value value byte array corresponding \b key
 * \param[out] size size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerGetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, void *value, nanoem_u32_t *size,
    nanoem_i32_t *status);

/**
 * \brief Set the effect compiler option value
 *
 * \param plugin The opaque effect compiler plugin object
 * \param key key of \b nanoem_application_plugin_effect_option_t
 * \param value value byte array corresponding \b key
 * \param size size of \b value in byte unit
 * \param[out] status \b NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS is set if succeeded, otherwise sets the others
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerSetOption(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t key, const void *value, nanoem_u32_t size,
    nanoem_i32_t *status);

/**
 * \brief Get string array of all available extensions
 *
 * \param plugin The opaque effect compiler plugin object
 * \param size number of return value
 * \return string array of all available extensions
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API const char *const *APIENTRY nanoemApplicationPluginEffectCompilerGetAvailableExtensions(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u32_t *size);

/**
 * \brief Compile the effect and generate artifact from the file
 *
 * Artifact data must be encoded with \b fx9.effect.Effect in \e emapp/resources/protobuf/effect.proto and will be
 * destroyed with ::nanoemApplicationPluginEffectCompilerDestroyBinary
 *
 * \param plugin The opaque effect compiler plugin object
 * \param path The absolute path of input effect data file
 * \param[out] size size of return value in byte unit
 * \return Bytes array of artifact encoded with \b fx9.effect.Effect
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u8_t *APIENTRY nanoemApplicationPluginEffectCompilerCreateBinaryFromFile(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, nanoem_u32_t *size);

/**
 * \brief Compile the effect and generate artifact from memory
 *
 * Artifact data must be encoded with \b fx9.effect.Effect in \e emapp/resources/protobuf/effect.proto and will be
 * destroyed with ::nanoemApplicationPluginEffectCompilerDestroyBinary
 *
 * \param plugin The opaque effect compiler plugin object
 * \param source Byte array of input effect data
 * \param length size of \b source in byte unit
 * \param[out] size size of return value in byte unit
 * \return Bytes array of artifact encoded with \b fx9.effect.Effect
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API nanoem_u8_t *APIENTRY nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *source, nanoem_u32_t length, nanoem_u32_t *size);

/**
 * \brief Adds \b #include directive source data corresponding path
 *
 * The function is intended to supply ::nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory and realize pseudo
 * file system.
 *
 * \param plugin The opaque effect compiler plugin object
 * \param path value of \b #include directive
 * \param data Byte array of \b path
 * \param size size of \b data in byte unit
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerAddIncludeSource(
    nanoem_application_plugin_effect_compiler_t *plugin, const char *path, const nanoem_u8_t *data, nanoem_u32_t size);

/**
 * \brief Get error failure reason text
 *
 * The function should be able to get failure reason when \b NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set
 * in status.
 *
 * \param plugin The opaque model plugin object
 * \return The error reason text to show it to the user via error dialog
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEffectCompilerGetFailureReason(
    const nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief Get error recovery suggestion text
 *
 * The function should be able to get recovery suggestion to resolve the issue by user when \b
 * NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON is set in status.
 *
 * \param plugin The opaque model plugin object
 * \return The recovery suggestion text to show it to the user via error dialog
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginEffectCompilerGetRecoverySuggestion(
    const nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief Destroy byte array of effect compiler artifact
 *
 * The function intends to destroy byte array of artifact returned from
 * ::nanoemApplicationPluginEffectCompilerCreateBinaryFromFile and
 * ::nanoemApplicationPluginEffectCompilerCreateBinaryFromMemory .
 *
 * \param plugin The opaque effect compiler plugin object
 * \param data Bytes array of artifact encoded with \b fx9.effect.Effect
 * \param size size of \b data in byte unit
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerDestroyBinary(
    nanoem_application_plugin_effect_compiler_t *plugin, nanoem_u8_t *data, nanoem_u32_t size);

/**
 * \brief Destroy an opaque effect compiler plugin object
 *
 * \param plugin The opaque effect compiler plugin object
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerDestroy(
    nanoem_application_plugin_effect_compiler_t *plugin);

/**
 * \brief Terminate the plugin
 *
 * \since Effect Compiler Plugin ABI 2.0
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginEffectCompilerTerminate(void);

/** @} */

/** @} */

#endif /* EMAPP_PLUGIN_SDK_EFFECT_H_ */
