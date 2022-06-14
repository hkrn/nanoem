/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_DEFAULT_FILE_MANAGER_H_
#define NANOEM_EMAPP_DEFAULT_FILE_MANAGER_H_

#include "emapp/DefaultTranslator.h"
#include "emapp/IFileManager.h"
#include "emapp/PluginFactory.h"

#include "bx/mutex.h"

namespace nanoem {

class Accessory;
class BaseApplicationService;
class FileWriterScope;
class IDrawable;
class IFileReader;
class IModalDialog;
class Model;
class Progress;
class StateController;

class DefaultFileManager : public IFileManager, private NonCopyable {
public:
    typedef tinystl::vector<plugin::ModelIOPlugin *, TinySTLAllocator> ModelIOPluginList;
    typedef tinystl::vector<plugin::MotionIOPlugin *, TinySTLAllocator> MotionIOPluginList;
    typedef tinystl::unordered_map<String, DecoderPluginList, TinySTLAllocator> ResolvedDecoderPluginListMap;
    typedef tinystl::unordered_map<String, EncoderPluginList, TinySTLAllocator> ResolvedEncoderPluginListMap;

    DefaultFileManager(BaseApplicationService *applicationPtr);
    ~DefaultFileManager() NANOEM_DECL_NOEXCEPT;

    void initializeAllDecoderPlugins(const URIList &fileURIs);
    void initializeAllAudioDecoderPlugins(const URIList &fileURIs, StringSet &availableExtensions);
    void initializeAllVideoDecoderPlugins(const URIList &fileURIs, StringSet &availableExtensions);
    void initializeAllEncoderPlugins(const URIList &fileURIs);
    void initializeAllEncoderPlugins(const URIList &fileURIs, StringSet &availableExtensions);
    void initializeAllModelIOPlugins(const URIList &fileURIs);
    void initializeAllMotionIOPlugins(const URIList &fileURIs);
    void cancelQueryFileDialog(Project *project);
    void destroyAllPlugins();

    bool loadAudioFile(const URI &fileURI, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool loadVideoFile(const URI &fileURI, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool loadProject(const URI &fileURI, Project *project, Error &error);

    DecoderPluginList resolveAudioDecoderPluginList(const char *extension) const NANOEM_DECL_OVERRIDE;
    DecoderPluginList resolveVideoDecoderPluginList(const char *extension) const NANOEM_DECL_OVERRIDE;
    EncoderPluginList allVideoEncoderPluginList() const NANOEM_DECL_OVERRIDE;
    ResolvedDecoderPluginListMap allResolvedAudioDecoderPlugins() const;
    ResolvedDecoderPluginListMap allResolvedVideoDecoderPlugins() const;
    void getAllModelIOPlugins(ModelIOPluginList &plugins);
    void getAllMotionIOPlugins(MotionIOPluginList &plugins);

protected:
    bool isVideoLoadable(Project *project, const URI &fileURI);
    URI sharedSourceEffectCacheDirectory() NANOEM_DECL_OVERRIDE;
    plugin::EffectPlugin *sharedEffectPlugin() NANOEM_DECL_OVERRIDE;

    StateController *stateController() NANOEM_DECL_NOEXCEPT;
    const BaseApplicationService *application() const NANOEM_DECL_NOEXCEPT;
    BaseApplicationService *application() NANOEM_DECL_NOEXCEPT;
    const ITranslator *translator() const NANOEM_DECL_NOEXCEPT;

    bool loadFromFile(const URI &fileURI, DialogType type, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool loadFromFileWithHandle(
        const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool loadFromFileWithModalDialog(
        const URI &fileURI, DialogType type, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool saveAsFile(const URI &fileURI, DialogType type, Project *project, Error &error) NANOEM_DECL_OVERRIDE;
    bool hasTransientQueryFileDialogCallback() const NANOEM_DECL_NOEXCEPT_OVERRIDE;
    void setTransientQueryFileDialogCallback(QueryFileDialogCallbacks callbacks) NANOEM_DECL_OVERRIDE;
    void resetTransientQueryFileDialogCallback() NANOEM_DECL_OVERRIDE;

private:
    typedef void (*LoadEffectCallback)(Effect *effect, Project *project, Error &error);
    static IModalDialog *handleAddingModelDialog(void *userData, Model *model, Project *project);
    static IModalDialog *loadEffectAfterEnablingEffectPluginDialog(
        void *userData, const URI &fileURI, nanoem_u16_t handle, int type, Project *project);
    static IModalDialog *loadModelAfterEnablingEffectPluginDialog(
        void *userData, const URI &fileURI, nanoem_u16_t handle, int type, Project *project);

    void loadModelEffectSettingFile(Model *model, Project *project, Progress &progress, Error &error);
    bool internalLoadFromFile(const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project,
        bool ignoreModalDialog, Error &error);
    bool internalLoadEffectSourceFile(LoadEffectCallback callback, const URI &fileURI, Project *project, Error &error);
    bool loadPlainText(const URI &fileURI, Project *project, Error &error);
    bool loadModel(const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project, Error &error);
    bool loadCameraMotion(const URI &fileURI, Project *project, Error &error);
    bool loadLightMotion(const URI &fileURI, Project *project, Error &error);
    bool loadModelMotion(const URI &fileURI, Project *project, Error &error);
    bool loadMotion(const URI &fileURI, Motion *motion, nanoem_frame_index_t offset, Error &error);
    bool loadEffectSource(const URI &fileURI, Project *project, Error &error);
    bool internalLoadAccessory(const URI &fileURI, Project *project, Error &error);
    bool internalLoadModel(const URI &fileURI, DialogType type, Project *project, Error &error);
    bool internalLoadAccessoryFromFile(const URI &fileURI, Accessory *accessory, Progress &progress, Error &error);
    bool internalLoadModelFromFile(const URI &fileURI, Model *model, Error &error);
    bool loadModelFromArchive(IFileReader *reader, const URI &fileURI, DialogType type, Project *project, Error &error);
    bool saveProject(const URI &fileURI, Project *project, Error &error);
    bool saveModel(const URI &fileURI, Project *project, Error &error);
    bool saveCameraMotion(const URI &fileURI, Project *project, Error &error);
    bool saveLightMotion(const URI &fileURI, Project *project, Error &error);
    bool saveModelMotion(const URI &fileURI, Project *project, Error &error);
    bool validateWrittenFile(FileWriterScope &scope, const nanoem_u8_t *expectedChecksum, Error &error);
    bool decodeAudioWave(
        const Project *project, PluginFactory::DecoderPluginProxy &proxy, ByteArray &bytes, Error &error);
    bool isCameraLightMotion(const Motion *motion);

    BaseApplicationService *m_applicationPtr;
    DefaultTranslator m_translator;
    ResolvedDecoderPluginListMap m_resolvedDecodingAudioPluginListMap;
    ResolvedDecoderPluginListMap m_resolvedDecodingVideoPluginListMap;
    DecoderPluginList m_decoderPlugins;
    EncoderPluginList m_encoderPlugins;
    tinystl::pair<ModelIOPluginList, bx::Mutex> m_modelIOPlugins;
    tinystl::pair<MotionIOPluginList, bx::Mutex> m_motionIOPlugins;
    plugin::EffectPlugin *m_effectPlugin;
    QueryFileDialogCallbacks m_queryFileDialogCallbacks;
};

} /* namespace nanoem */

#endif /* NANOEM_EMAPP_DEFAULT_FILE_MANAGER_H_ */
