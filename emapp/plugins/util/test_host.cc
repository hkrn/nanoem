/*
   Copyright (c) 2015-2023 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include <stdio.h>

#define BX_CONFIG_CRT_FILE_READER_WRITER 1
#include "bx/crtimpl.h"
#include "bx/os.h"
#include "nanoem/ext/mutable.h"
#include "nanoem/nanoem.h"
#include "tinystl/allocator.h"
#include "tinystl/string.h"

namespace {

typedef int (*PFN_nanoemApplicationPluginResolveAllFunctionAddresses)(const char *);
PFN_nanoemApplicationPluginResolveAllFunctionAddresses nanoemApplicationPluginResolveAllFunctionAddresses = NULL;
struct nanoem_application_plugin_importer_t;
typedef void (*PFN_nanoemApplicationPluginImporterInitialize)(void);
PFN_nanoemApplicationPluginImporterInitialize nanoemApplicationPluginImporterInitialize = NULL;
typedef nanoem_application_plugin_importer_t *(*PFN_nanoemApplicationPluginImporterCreate)(
    nanoem_unicode_string_factory_t *);
PFN_nanoemApplicationPluginImporterCreate nanoemApplicationPluginImporterCreate = NULL;
typedef int (*PFN_nanoemApplicationPluginImporterSetOption)(
    nanoem_application_plugin_importer_t *, nanoem_u32_t, const void *);
PFN_nanoemApplicationPluginImporterSetOption nanoemApplicationPluginImporterSetOption = NULL;
typedef nanoem_model_t *(*PFN_nanoemApplicationPluginImporterImportModelFromFile)(
    nanoem_application_plugin_importer_t *, const char *);
PFN_nanoemApplicationPluginImporterImportModelFromFile nanoemApplicationPluginImporterImportModelFromFile = NULL;
typedef const char *(*PFN_nanoemApplicationPluginImporterGetFailureReason)(
    const nanoem_application_plugin_importer_t *);
PFN_nanoemApplicationPluginImporterGetFailureReason nanoemApplicationPluginImporterGetFailureReason = NULL;
typedef const char *(*PFN_nanoemApplicationPluginImporterGetRecoverySuggestion)(
    const nanoem_application_plugin_importer_t *);
PFN_nanoemApplicationPluginImporterGetRecoverySuggestion nanoemApplicationPluginImporterGetRecoverySuggestion = NULL;
typedef void (*PFN_nanoemApplicationPluginImporterDestroy)(nanoem_application_plugin_importer_t *);
PFN_nanoemApplicationPluginImporterDestroy nanoemApplicationPluginImporterDestroy = NULL;

static int
resolveAllFunctionAddresses(void *symbol)
{
    nanoemApplicationPluginResolveAllFunctionAddresses =
        reinterpret_cast<PFN_nanoemApplicationPluginResolveAllFunctionAddresses>(
            bx::dlsym(symbol, "nanoemApplicationPluginResolveAllFunctionAddresses"));
    int ret = nanoemApplicationPluginResolveAllFunctionAddresses("libnanoem_plugin_test_dso.dylib");
    if (ret == NANOEM_STATUS_SUCCESS) {
        if ((nanoemApplicationPluginImporterInitialize =
                    reinterpret_cast<PFN_nanoemApplicationPluginImporterInitialize>(
                        bx::dlsym(symbol, "nanoemApplicationPluginImporterInitialize"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterCreate = reinterpret_cast<PFN_nanoemApplicationPluginImporterCreate>(
                 bx::dlsym(symbol, "nanoemApplicationPluginImporterCreate"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterSetOption = reinterpret_cast<PFN_nanoemApplicationPluginImporterSetOption>(
                 bx::dlsym(symbol, "nanoemApplicationPluginImporterSetOption"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterImportModelFromFile =
                    reinterpret_cast<PFN_nanoemApplicationPluginImporterImportModelFromFile>(
                        bx::dlsym(symbol, "nanoemApplicationPluginImporterImportModelFromFile"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterGetFailureReason =
                    reinterpret_cast<PFN_nanoemApplicationPluginImporterGetFailureReason>(
                        bx::dlsym(symbol, "nanoemApplicationPluginImporterGetFailureReason"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterGetRecoverySuggestion =
                    reinterpret_cast<PFN_nanoemApplicationPluginImporterGetRecoverySuggestion>(
                        bx::dlsym(symbol, "nanoemApplicationPluginImporterGetRecoverySuggestion"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
        if ((nanoemApplicationPluginImporterDestroy = reinterpret_cast<PFN_nanoemApplicationPluginImporterDestroy>(
                 bx::dlsym(symbol, "nanoemApplicationPluginImporterDestroy"))) == NULL) {
            return NANOEM_STATUS_UNKNOWN;
        }
    }
    return ret;
}
}

int
main(int argc, char *argv[])
{
    if (argc >= 2) {
        const char *selfPath = argv[0];
        const char *dllPath = argv[1];
        const char *inputPath = argc >= 3 ? argv[2] : "./test.obj";
        fprintf(stderr, "version: %s\n", nanoemGetVersionString());
        fprintf(stderr, "selfPath: %s\n", selfPath);
        fprintf(stderr, "dllPath: %s\n", dllPath);
        fprintf(stderr, "inputPath: %s\n", inputPath);
        if (void *symbol = bx::dlopen(dllPath)) {
            ;
            int ret = resolveAllFunctionAddresses(symbol);
            if (ret == NANOEM_STATUS_SUCCESS) {
                nanoemApplicationPluginImporterInitialize();
                nanoem_unicode_string_factory_t *factory = nanoemUnicodeStringFactoryCreate();
                nanoem_application_plugin_importer_t *importer = nanoemApplicationPluginImporterCreate(factory);
                if (nanoem_model_t *model = nanoemApplicationPluginImporterImportModelFromFile(importer, inputPath)) {
                    nanoem_mutable_status_t status;
                    nanoem_mutable_model_t *mutableModel = nanoemMutableModelCreateAsReference(model);
                    nanoem_mutable_buffer_t *mutableBuffer = nanoemMutableBufferCreate();
                    if (nanoemMutableModelSaveToBuffer(mutableModel, mutableBuffer, &status)) {
                        nanoem_buffer_t *buffer = nanoemMutableBufferCreateBufferObject(mutableBuffer);
                        bx::CrtFileWriter writer;
                        tinystl::string outputPath;
                        if (const char *p = strrchr(inputPath, '/')) {
                            outputPath.append(inputPath, p);
                            outputPath.append("/output.pmx");
                        }
                        else {
                            outputPath.append("output.pmx");
                        }
                        if (bx::open(&writer, outputPath.c_str(), false)) {
                            bx::write(&writer, nanoemBufferGetDataPtr(buffer), nanoemBufferGetLength(buffer));
                            bx::close(&writer);
                            fprintf(stderr, "complete writing to %s\n", outputPath.c_str());
                        }
                        nanoemBufferDestroy(buffer);
                    }
                    nanoemMutableBufferDestroy(mutableBuffer);
                    nanoemMutableModelDestroy(mutableModel);
                    nanoemModelDestroy(model);
                }
                else {
                    fprintf(stderr, "reason: %s\n", nanoemApplicationPluginImporterGetFailureReason(importer));
                    fprintf(stderr, "recovery: %s\n", nanoemApplicationPluginImporterGetRecoverySuggestion(importer));
                }
                nanoemApplicationPluginImporterDestroy(importer);
                nanoemUnicodeStringFactoryDestroy(factory);
            }
            bx::dlclose(symbol);
        }
    }
    return 0;
}
