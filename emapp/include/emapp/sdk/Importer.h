/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#ifndef EMAPP_PLUGIN_SDK_IMPORTER_H_
#define EMAPP_PLUGIN_SDK_IMPORTER_H_

#define NANOEM_APPLICATION_PLUGIN_IMPORTER_ABI_VERSION_MAJOR 2
#define NANOEM_APPLICATION_PLUGIN_IMPORTER_ABI_VERSION_MINOR 0
#define NANOEM_APPLICATION_PLUGIN_IMPORTER_ABI_VERSION                                                                 \
    NANOEM_APPLICATION_PLUGIN_MAKE_ABI_VERSION(                                                                        \
        NANOEM_APPLICATION_PLUGIN_IMPORTER_ABI_VERSION_MAJOR, NANOEM_APPLICATION_PLUGIN_IMPORTER_ABI_VERSION_MINOR)

#include "Common.h"

typedef struct nanoem_application_plugin_importer_t nanoem_application_plugin_importer_t;
typedef struct nanoem_unicode_string_factory_t nanoem_unicode_string_factory_t;
typedef struct nanoem_model_t nanoem_model_t;
typedef struct nanoem_motion_t nanoem_motion_t;

enum {
    NANOEM_APPLICATION_PLUGIN_IMPORTER_OPTION_UNKNOWN = -1,
    NANOEM_APPLICATION_PLUGIN_IMPORTER_OPTION_FIRST_ENUM,
    NANOEM_APPLICATION_PLUGIN_IMPORTER_OPTION_MAX_ENUM
};

/**
 * \brief
 *
 * \return NANOEM_DECL_API nanoem_u32_t APIENTRY
 */
NANOEM_DECL_API nanoem_u32_t APIENTRY nanoemApplicationPluginImporterGetABIVersion(void);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginImporterInitialize(void);

/**
 * \brief
 *
 * \param factory
 * \return NANOEM_DECL_API nanoem_application_plugin_importer_t* APIENTRY
 */
NANOEM_DECL_API nanoem_application_plugin_importer_t *APIENTRY nanoemApplicationPluginImporterCreate(
    nanoem_unicode_string_factory_t *factory);

/**
 * \brief
 *
 * \param plugin
 * \param key
 * \param value
 * \param size
 * \return NANOEM_DECL_API int APIENTRY
 */
NANOEM_DECL_API int APIENTRY nanoemApplicationPluginImporterSetOption(
    nanoem_application_plugin_importer_t *plugin, nanoem_u32_t key, const void *value, nanoem_rsize_t size);

/**
 * \brief
 *
 * \param plugin
 * \param path
 * \return NANOEM_DECL_API nanoem_model_t* APIENTRY
 */
NANOEM_DECL_API nanoem_model_t *APIENTRY nanoemApplicationPluginImporterImportModelFromFile(
    nanoem_application_plugin_importer_t *plugin, const char *path);

/**
 * \brief
 *
 * \param plugin
 * \param path
 * \return NANOEM_DECL_API nanoem_motion_t* APIENTRY
 */
NANOEM_DECL_API nanoem_motion_t *APIENTRY nanoemApplicationPluginImporterImportMotionFromFile(
    nanoem_application_plugin_importer_t *plugin, const char *path);

/**
 * \brief
 *
 * \param plugin
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginImporterGetFailureReason(
    const nanoem_application_plugin_importer_t *plugin);

/**
 * \brief
 *
 * \param plugin
 * \return NANOEM_DECL_API const char* APIENTRY
 */
NANOEM_DECL_API const char *APIENTRY nanoemApplicationPluginImporterGetRecoverySuggestion(
    const nanoem_application_plugin_importer_t *plugin);

/**
 * \brief
 *
 * \param plugin
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginImporterDestroy(nanoem_application_plugin_importer_t *plugin);

/**
 * \brief
 *
 */
NANOEM_DECL_API void APIENTRY nanoemApplicationPluginImporterTerminate(void);

#endif /* EMAPP_PLUGIN_SDK_IMPORTER_H_ */
