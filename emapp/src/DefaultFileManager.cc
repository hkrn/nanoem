/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "emapp/DefaultFileManager.h"
#include "emapp/Accessory.h"
#include "emapp/Archiver.h"
#include "emapp/BaseApplicationService.h"
#include "emapp/BaseAudioPlayer.h"
#include "emapp/Effect.h"
#include "emapp/FileUtils.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IEventPublisher.h"
#include "emapp/IFileManager.h"
#include "emapp/IModalDialog.h"
#include "emapp/IMotionKeyframeSelection.h"
#include "emapp/ImageLoader.h"
#include "emapp/ModalDialogFactory.h"
#include "emapp/Model.h"
#include "emapp/Motion.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ResourceBundle.h"
#include "emapp/StringUtils.h"
#include "emapp/internal/ModelEffectSetting.h"
#include "emapp/plugin/DecoderPlugin.h"
#include "emapp/plugin/EncoderPlugin.h"
#include "emapp/private/CommonInclude.h"

#include "bx/handlealloc.h"
#include "protoc/application.pb-c.h"

#include <ctype.h> /* isspace */
#if !BX_PLATFORM_WINDOWS
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* BX_PLATFORM_WINDOWS */

namespace nanoem {

#include "sha256.h"

namespace {

struct VACData {
    VACData(nanoem_unicode_string_factory_t *factory);
    ~VACData() NANOEM_DECL_NOEXCEPT;

    bool read(const URI &fileURI, Error &error);
    static const char *skipSpaceOrComma(const char *p) NANOEM_DECL_NOEXCEPT;

    nanoem_unicode_string_factory_t *m_factory;
    String m_name;
    String m_filename;
    nanoem_f32_t m_scaleFactor;
    Vector3 m_translation;
    Vector3 m_orientation;
    String m_boneName;
};

VACData::VACData(nanoem_unicode_string_factory_t *factory)
    : m_factory(factory)
    , m_scaleFactor(0)
    , m_translation(0)
    , m_orientation(0)
{
}

VACData::~VACData() NANOEM_DECL_NOEXCEPT
{
}

bool
VACData::read(const URI &fileURI, Error &error)
{
    FileReaderScope scope(nullptr);
    bool result = false;
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        if (!error.hasReason()) {
            const nanoem_u8_t *dataPtr = bytes.data();
            nanoem_rsize_t offset = 0, dataSize = bytes.size();
            MutableString s;
            int numLines = 0;
            while (offset < dataSize) {
                char c = *reinterpret_cast<const char *>(dataPtr + offset);
                switch (c) {
                case '\r': {
                    /* ignore */
                    break;
                }
                case '\n': {
                    switch (numLines) {
                    case 0: {
                        StringUtils::getUtf8String(s.data(), s.size(), NANOEM_CODEC_TYPE_SJIS, m_factory, m_name);
                        break;
                    }
                    case 1: {
                        StringUtils::getUtf8String(s.data(), s.size(), NANOEM_CODEC_TYPE_SJIS, m_factory, m_filename);
                        break;
                    }
                    case 2: {
                        m_scaleFactor = StringUtils::parseFloat(s.data(), nullptr);
                        break;
                    }
                    case 3: {
                        const char *ptr = s.data();
                        char *p = nullptr;
                        m_translation.x = StringUtils::parseFloat(ptr, &p);
                        ptr = skipSpaceOrComma(p);
                        m_translation.y = StringUtils::parseFloat(ptr, &p);
                        ptr = skipSpaceOrComma(p);
                        m_translation.z = StringUtils::parseFloat(ptr, &p);
                        break;
                    }
                    case 4: {
                        const char *ptr = s.data();
                        char *p = nullptr;
                        m_orientation.x = StringUtils::parseFloat(ptr, &p);
                        ptr = skipSpaceOrComma(p);
                        m_orientation.y = StringUtils::parseFloat(ptr, &p);
                        ptr = skipSpaceOrComma(p);
                        m_orientation.z = StringUtils::parseFloat(ptr, &p);
                        break;
                    }
                    case 5: {
                        StringUtils::getUtf8String(s.data(), s.size(), NANOEM_CODEC_TYPE_SJIS, m_factory, m_boneName);
                        break;
                    }
                    default:
                        result = true;
                        break;
                    }
                    numLines++;
                    s.clear();
                    break;
                }
                default:
                    s.push_back(c);
                    break;
                }
                offset++;
            }
        }
    }
    return result;
}

const char *
VACData::skipSpaceOrComma(const char *p) NANOEM_DECL_NOEXCEPT
{
    while (isspace(*p) || *p == ',') {
        p++;
    }
    return p;
}

static void
setEffectCallback(Effect *effect, Project *project, Error &error)
{
    project->attachEffectToSelectedDrawable(effect, error);
}

} /* namespace anonymous */

DefaultFileManager::DefaultFileManager(BaseApplicationService *applicationPtr)
    : m_applicationPtr(applicationPtr)
    , m_effectPlugin(nullptr)
{
    const JSON_Object *config = json_object(applicationPtr->applicationConfiguration());
    const char *locale = json_object_dotget_string(config, "project.locale");
    if (locale && StringUtils::equals(locale, "ja_JP", 5)) {
        m_translator.setLanguage(ITranslator::kLanguageTypeJapanese);
    }
    else {
        m_translator.setLanguage(ITranslator::kLanguageTypeEnglish);
    }
    m_applicationPtr->setTranslator(&m_translator);
    Inline::clearZeroMemory(m_queryFileDialogCallbacks);
}

DefaultFileManager::~DefaultFileManager() NANOEM_DECL_NOEXCEPT
{
    destroyAllPlugins();
}

void
DefaultFileManager::initializeAllDecoderPlugins(const URIList &fileURIs)
{
    if (m_decoderPlugins.empty()) {
        IEventPublisher *publisher = m_applicationPtr->eventPublisher();
        for (URIList::const_iterator it = fileURIs.begin(), end = fileURIs.end(); it != end; ++it) {
            if (plugin::DecoderPlugin *plugin = PluginFactory::createDecoderPlugin(*it, publisher)) {
                const PluginFactory::DecoderPluginProxy proxy(plugin);
                const StringList &audioExtensions = proxy.availableDecodingAudioExtensions();
                const StringList &videoExtensions = proxy.availableDecodingVideoExtensions();
                if (!audioExtensions.empty() || !videoExtensions.empty()) {
                    for (StringList::const_iterator it2 = audioExtensions.begin(), end2 = audioExtensions.end();
                         it2 != end2; ++it2) {
                        m_resolvedDecodingAudioPluginListMap[*it2].push_back(plugin);
                    }
                    for (StringList::const_iterator it2 = videoExtensions.begin(), end2 = videoExtensions.end();
                         it2 != end2; ++it2) {
                        m_resolvedDecodingVideoPluginListMap[*it2].push_back(plugin);
                    }
                    m_decoderPlugins.push_back(plugin);
                }
                else {
                    PluginFactory::destroyDecoderPlugin(plugin);
                }
            }
        }
    }
}

void
DefaultFileManager::initializeAllAudioDecoderPlugins(const URIList &fileURIs, StringSet &availableExtensions)
{
    StringList extenstions;
    initializeAllDecoderPlugins(fileURIs);
    availableExtensions.clear();
    for (DecoderPluginList::const_iterator it = m_decoderPlugins.begin(), end = m_decoderPlugins.end(); it != end;
         ++it) {
        plugin::DecoderPlugin *plugin = *it;
        plugin->getAvailableAudioFormatExtensions(extenstions);
        for (StringList::const_iterator it2 = extenstions.begin(), end2 = extenstions.end(); it2 != end2; it2++) {
            availableExtensions.insert(*it2);
        }
    }
}

void
DefaultFileManager::initializeAllVideoDecoderPlugins(const URIList &fileURIs, StringSet &availableExtensions)
{
    StringList extenstions;
    initializeAllDecoderPlugins(fileURIs);
    availableExtensions.clear();
    for (DecoderPluginList::const_iterator it = m_decoderPlugins.begin(), end = m_decoderPlugins.end(); it != end;
         ++it) {
        plugin::DecoderPlugin *plugin = *it;
        plugin->getAvailableVideoFormatExtensions(extenstions);
        for (StringList::const_iterator it2 = extenstions.begin(), end2 = extenstions.end(); it2 != end2; it2++) {
            availableExtensions.insert(*it2);
        }
    }
}

void
DefaultFileManager::initializeAllEncoderPlugins(const URIList &fileURIs)
{
    if (m_encoderPlugins.empty()) {
        IEventPublisher *publisher = m_applicationPtr->eventPublisher();
        for (URIList::const_iterator it = fileURIs.begin(), end = fileURIs.end(); it != end; ++it) {
            if (plugin::EncoderPlugin *plugin = PluginFactory::createEncoderPlugin(*it, publisher)) {
                m_encoderPlugins.push_back(plugin);
            }
        }
    }
}

void
DefaultFileManager::initializeAllEncoderPlugins(const URIList &fileURIs, StringSet &availableExtensions)
{
    StringList extenstions;
    initializeAllEncoderPlugins(fileURIs);
    availableExtensions.clear();
    for (EncoderPluginList::const_iterator it = m_encoderPlugins.begin(), end = m_encoderPlugins.end(); it != end;
         ++it) {
        plugin::EncoderPlugin *plugin = *it;
        plugin->getAvailableVideoFormatExtensions(extenstions);
        for (StringList::const_iterator it2 = extenstions.begin(), end2 = extenstions.end(); it2 != end2; it2++) {
            availableExtensions.insert(*it2);
        }
    }
}

void
DefaultFileManager::initializeAllModelIOPlugins(const URIList &fileURIs)
{
    if (!fileURIs.empty()) {
        bx::MutexScope locker(m_modelIOPlugins.second);
        ModelIOPluginList &plugins = m_modelIOPlugins.first;
        if (plugins.empty()) {
            IEventPublisher *publisher = m_applicationPtr->eventPublisher();
            for (URIList::const_iterator it = fileURIs.begin(), end = fileURIs.end(); it != end; ++it) {
                if (plugin::ModelIOPlugin *plugin = PluginFactory::createModelIOPlugin(*it, publisher)) {
                    plugins.push_back(plugin);
                }
            }
        }
    }
}

void
DefaultFileManager::initializeAllMotionIOPlugins(const URIList &fileURIs)
{
    if (!fileURIs.empty()) {
        bx::MutexScope locker(m_motionIOPlugins.second);
        MotionIOPluginList &plugins = m_motionIOPlugins.first;
        if (plugins.empty()) {
            IEventPublisher *publisher = m_applicationPtr->eventPublisher();
            for (URIList::const_iterator it = fileURIs.begin(), end = fileURIs.end(); it != end; ++it) {
                if (plugin::MotionIOPlugin *plugin = PluginFactory::createMotionIOPlugin(*it, publisher)) {
                    plugins.push_back(plugin);
                }
            }
        }
    }
}

void
DefaultFileManager::cancelQueryFileDialog(Project *project)
{
    if (m_queryFileDialogCallbacks.m_cancel) {
        m_queryFileDialogCallbacks.m_cancel(project, m_queryFileDialogCallbacks.m_opaque);
        m_queryFileDialogCallbacks.m_cancel = nullptr;
    }
    resetTransientQueryFileDialogCallback();
}

void
DefaultFileManager::destroyAllPlugins()
{
    for (DecoderPluginList::const_iterator it = m_decoderPlugins.begin(), end = m_decoderPlugins.end(); it != end;
         ++it) {
        PluginFactory::destroyDecoderPlugin(*it);
    }
    m_decoderPlugins.clear();
    for (EncoderPluginList::const_iterator it = m_encoderPlugins.begin(), end = m_encoderPlugins.end(); it != end;
         ++it) {
        PluginFactory::destroyEncoderPlugin(*it);
    }
    m_encoderPlugins.clear();
    {
        bx::MutexScope locker(m_modelIOPlugins.second);
        ModelIOPluginList &plugins = m_modelIOPlugins.first;
        for (ModelIOPluginList::const_iterator it = plugins.begin(), end = plugins.end(); it != end; ++it) {
            PluginFactory::destroyModelIOPlugin(*it);
        }
        plugins.clear();
    }
    {
        bx::MutexScope locker(m_motionIOPlugins.second);
        MotionIOPluginList &plugins = m_motionIOPlugins.first;
        for (MotionIOPluginList::const_iterator it = plugins.begin(), end = plugins.end(); it != end; ++it) {
            PluginFactory::destroyMotionIOPlugin(*it);
        }
        plugins.clear();
    }
    if (m_effectPlugin) {
        PluginFactory::destroyEffectPlugin(m_effectPlugin);
        m_effectPlugin = nullptr;
    }
}

IFileManager::DecoderPluginList
DefaultFileManager::resolveAudioDecoderPluginList(const char *extension) const
{
    ResolvedDecoderPluginListMap::const_iterator it = m_resolvedDecodingAudioPluginListMap.find(extension);
    return it != m_resolvedDecodingAudioPluginListMap.end() ? it->second : DecoderPluginList();
}

IFileManager::DecoderPluginList
DefaultFileManager::resolveVideoDecoderPluginList(const char *extension) const
{
    ResolvedDecoderPluginListMap::const_iterator it = m_resolvedDecodingVideoPluginListMap.find(extension);
    return it != m_resolvedDecodingVideoPluginListMap.end() ? it->second : DecoderPluginList();
}

IFileManager::EncoderPluginList
DefaultFileManager::allVideoEncoderPluginList() const
{
    return m_encoderPlugins;
}

DefaultFileManager::ResolvedDecoderPluginListMap
DefaultFileManager::allResolvedAudioDecoderPlugins() const
{
    return m_resolvedDecodingAudioPluginListMap;
}

DefaultFileManager::ResolvedDecoderPluginListMap
DefaultFileManager::allResolvedVideoDecoderPlugins() const
{
    return m_resolvedDecodingVideoPluginListMap;
}

void
DefaultFileManager::getAllModelIOPlugins(ModelIOPluginList &plugins)
{
    bx::MutexScope locker(m_modelIOPlugins.second);
    plugins = m_modelIOPlugins.first;
}

void
DefaultFileManager::getAllMotionIOPlugins(MotionIOPluginList &plugins)
{
    bx::MutexScope locker(m_motionIOPlugins.second);
    plugins = m_motionIOPlugins.first;
}

bool
DefaultFileManager::loadAudioFile(const URI &fileURI, Project *project, Error &error)
{
    nanoem_parameter_assert(project, "project must not be nullptr");
    bool succeeded = false;
    IFileManager::DecoderPluginList plugins(resolveAudioDecoderPluginList(fileURI.pathExtension().c_str()));
    for (IFileManager::DecoderPluginList::const_iterator it = plugins.begin(), end = plugins.end(); it != end; ++it) {
        plugin::DecoderPlugin *plugin = *it;
        PluginFactory::DecoderPluginProxy proxy(plugin);
        if (proxy.loadAudio(fileURI, Constants::kHalfBaseFPS, error)) {
            project->setBaseDuration(proxy.duration());
            ByteArray bytes;
            IAudioPlayer *audio = project->audioPlayer();
            if (decodeAudioWave(project, proxy, bytes, error)) {
                IAudioPlayer::WAVDescription desc;
                BaseAudioPlayer::initializeDescription(
                    proxy.numBits(), proxy.numChannels(), proxy.frequency(), bytes.size(), desc);
                if (audio->load(bytes, desc, error)) {
                    audio->setFileURI(fileURI);
                    project->setBaseDuration(audio);
                    succeeded = true;
                    break;
                }
            }
        }
    }
    if (!succeeded) {
        if (BaseAudioPlayer::isLoadableExtension(fileURI)) {
            FileReaderScope scope(&m_translator);
            if (scope.open(fileURI, error)) {
                ByteArray bytes;
                FileUtils::read(scope, bytes, error);
                if (!error.hasReason()) {
                    IAudioPlayer *audio = project->audioPlayer();
                    if (audio->load(bytes, error)) {
                        audio->setFileURI(fileURI);
                        project->setBaseDuration(audio);
                        succeeded = true;
                    }
                }
            }
        }
        else if (!error.hasReason()) {
            error = Error("No decodable audio plugin found", 0, Error::kDomainTypeApplication);
        }
    }
    return succeeded;
}

bool
DefaultFileManager::loadVideoFile(const URI &fileURI, Project *project, Error &error)
{
    bool succeeded = false;
    if (!fileURI.isEmpty()) {
        succeeded = project->backgroundVideoRenderer()->load(fileURI, error);
    }
    return succeeded;
}

bool
DefaultFileManager::loadProject(const URI &fileURI, Project *project, Error &error)
{
    FileReaderScope scope(&m_translator);
    bool succeeded = false;
    if (scope.open(fileURI, error)) {
        if (Project::isArchiveURI(fileURI)) {
            succeeded = project->loadFromArchive(scope.reader(), fileURI, error);
        }
        else {
            const String extension(fileURI.pathExtension());
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            if (!error.hasReason()) {
                project->setFileURI(fileURI);
                struct Diagnostics : Project::IDiagnostics {
                    void
                    addNotFoundFileURI(const URI &fileURI) NANOEM_DECL_OVERRIDE
                    {
                        m_fileURIsNotFound.push_back(fileURI);
                    }
                    void
                    addDigestMismatchFileURI(const URI &fileURI) NANOEM_DECL_OVERRIDE
                    {
                        m_fileURIsMismatch.push_back(fileURI);
                    }
                    void
                    addModalDialog(BaseApplicationService *service)
                    {
                        if (!m_fileURIsNotFound.empty() || !m_fileURIsMismatch.empty()) {
                            String message;
                            const ITranslator *translator = service->translator();
                            StringUtils::format(
                                message, "%s\n\n", translator->translate("nanoem.project.diagnostics.message.main"));
                            if (!m_fileURIsNotFound.empty()) {
                                StringUtils::format(message, "%s\n",
                                    translator->translate("nanoem.project.diagnostics.message.not-found"));
                                for (URIList::const_iterator it = m_fileURIsNotFound.begin(),
                                                             end = m_fileURIsNotFound.end();
                                     it != end; ++it) {
                                    message.append(it->absolutePathConstString());
                                    message.append("\n");
                                }
                                message.append("\n");
                            }
                            if (!m_fileURIsMismatch.empty()) {
                                StringUtils::format(message, "%s\n",
                                    translator->translate("nanoem.project.diagnostics.message.digest-mismatch"));
                                for (URIList::const_iterator it = m_fileURIsMismatch.begin(),
                                                             end = m_fileURIsMismatch.end();
                                     it != end; ++it) {
                                    message.append(it->absolutePathConstString());
                                    message.append("\n");
                                }
                            }
                            IModalDialog *dialog = ModalDialogFactory::createDisplayPlainTextDialog(
                                service, translator->translate("nanoem.project.diagnostics.title"), message);
                            service->addModalDialog(dialog);
                        }
                    }
                    URIList m_fileURIsNotFound;
                    URIList m_fileURIsMismatch;
                };
                Diagnostics diagnostics;
                if ((StringUtils::equals(extension.c_str(), Project::kFileSystemBasedNativeFormatFileExtension) &&
                        project->loadFromBinary(bytes, Project::kBinaryFormatNative, error, &diagnostics)) ||
                    (StringUtils::equals(extension.c_str(), Project::kPolygonMovieMakerFileExtension) &&
                        project->loadFromBinary(bytes, Project::kBinaryFormatPMM, error, &diagnostics))) {
                    succeeded = !error.isCancelled();
                    diagnostics.addModalDialog(m_applicationPtr);
                }
            }
        }
    }
    if (succeeded) {
        FileUtils::TransientPath lastTransientPath;
        if (project->hasTransientPath()) {
            lastTransientPath = project->transientPath();
        }
        FileUtils::TransientPath transientPath;
        if (FileUtils::createTransientFile(fileURI.absolutePath(), transientPath)) {
            project->setTransientPath(transientPath);
        }
        project->setFileURI(fileURI);
        project->writeRedoMessage();
        if (lastTransientPath.m_valid) {
            FileUtils::deleteTransientFile(lastTransientPath);
        }
        succeeded = !error.hasReason();
    }
    return succeeded;
}

bool
DefaultFileManager::hasTransientQueryFileDialogCallback() const NANOEM_DECL_NOEXCEPT
{
    return m_queryFileDialogCallbacks.m_accept != nullptr;
}

void
DefaultFileManager::setTransientQueryFileDialogCallback(QueryFileDialogCallbacks callbacks)
{
    m_queryFileDialogCallbacks = callbacks;
}

void
DefaultFileManager::resetTransientQueryFileDialogCallback()
{
    if (m_queryFileDialogCallbacks.m_destory) {
        m_queryFileDialogCallbacks.m_destory(m_queryFileDialogCallbacks.m_opaque);
        m_queryFileDialogCallbacks.m_destory = nullptr;
    }
    Inline::clearZeroMemory(m_queryFileDialogCallbacks);
}

bool
DefaultFileManager::isVideoLoadable(Project * /* project */, const URI &fileURI)
{
    static const String kPathExtensionBMP("bmp"), kPathExtensionJPG("jpg"), kPathExtensionPNG("png");
    DecoderPluginList plugins(resolveVideoDecoderPluginList(fileURI.pathExtension().c_str()));
    bool loadable = !plugins.empty();
    if (!loadable) {
        const String &pathExtension = fileURI.pathExtension();
        loadable = pathExtension == kPathExtensionBMP || pathExtension == kPathExtensionJPG ||
            pathExtension == kPathExtensionPNG;
    }
    return loadable;
}

URI
DefaultFileManager::sharedSourceEffectCacheDirectory()
{
    const JSON_Object *config = json_object(m_applicationPtr->applicationConfiguration());
    URI directoryURI;
    if (const char *path = json_object_dotget_string(config, "plugin.effect.cache.path")) {
        directoryURI = URI::createFromFilePath(path);
    }
    return directoryURI;
}

plugin::EffectPlugin *
DefaultFileManager::sharedEffectPlugin()
{
    Error error;
    if (!m_effectPlugin) {
        const JSON_Object *config = json_object(m_applicationPtr->applicationConfiguration());
        if (const char *filePath = json_object_dotget_string(config, "plugin.effect.path")) {
            const URI &fileURI = URI::createFromFilePath(filePath);
            IEventPublisher *publisher = m_applicationPtr->eventPublisher();
            m_effectPlugin = PluginFactory::createEffectPlugin(fileURI, publisher, error);
        }
    }
    return m_effectPlugin;
}

const BaseApplicationService *
DefaultFileManager::application() const NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr;
}

BaseApplicationService *
DefaultFileManager::application() NANOEM_DECL_NOEXCEPT
{
    return m_applicationPtr;
}

const ITranslator *
DefaultFileManager::translator() const NANOEM_DECL_NOEXCEPT
{
    return &m_translator;
}

bool
DefaultFileManager::loadFromFile(const URI &fileURI, DialogType type, Project *project, Error &error)
{
    return internalLoadFromFile(fileURI, bx::kInvalidHandle, type, project, true, error);
}

bool
DefaultFileManager::loadFromFileWithHandle(
    const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project, Error &error)
{
    return internalLoadFromFile(fileURI, handle, type, project, true, error);
}

bool
DefaultFileManager::loadFromFileWithModalDialog(const URI &fileURI, DialogType type, Project *project, Error &error)
{
    return internalLoadFromFile(fileURI, bx::kInvalidHandle, type, project, false, error);
}

void
DefaultFileManager::loadModelEffectSettingFile(Model *model, Project *project, Progress &progress, Error &error)
{
    const URI fileURI(model->fileURI());
    String emdPath(fileURI.absolutePathByDeletingPathExtension());
    emdPath.append(".emd");
    if (FileUtils::exists(emdPath.c_str())) {
        FileReaderScope scope(translator());
        if (scope.open(URI::createFromFilePath(emdPath), error)) {
            ByteArray bytes;
            internal::ModelEffectSetting loader(project, this);
            FileUtils::read(scope, bytes, error);
            bytes.push_back(0);
            loader.load(reinterpret_cast<const char *>(bytes.data()), model, progress, error);
        }
    }
}

bool
DefaultFileManager::internalLoadFromFile(
    const URI &fileURI, nanoem_u16_t handle, DialogType type, Project *project, bool ignoreModalDialog, Error &error)
{
    nanoem_parameter_assert(project, "must NOT be nullptr");
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    bool succeeded = false;
    if (!ignoreModalDialog && m_applicationPtr->currentModalDialog()) {
        /* do nothing */
        succeeded = true;
    }
    else if (type != kDialogTypeUserCallback && fileURI.pathExtension() == String("txt")) {
        succeeded = loadPlainText(fileURI, project, error);
    }
    else if (project) {
        switch (type) {
        case kDialogTypeOpenProject: {
            succeeded = loadProject(fileURI, project, error);
            break;
        }
        case kDialogTypeOpenModelFile: {
            bool needsConfirmDialog = false;
            if (!project->isEffectPluginEnabled()) {
                const String baseEffectPath(fileURI.absolutePathByDeletingPathExtension());
                const char *const kSearchExtensions[] = { "emd", Effect::kSourceFileExtension,
                    Effect::kBinaryFileExtension };
                bool effectFileExists = false;
                for (nanoem_rsize_t i = 0; i < BX_COUNTOF(kSearchExtensions); i++) {
                    String path(baseEffectPath);
                    path.append(".");
                    path.append(kSearchExtensions[i]);
                    effectFileExists = FileUtils::exists(path.c_str());
                    if (effectFileExists) {
                        break;
                    }
                }
                if (effectFileExists) {
                    const ModalDialogFactory::EnablingEffectPluginDialogCallback callback(
                        loadModelAfterEnablingEffectPluginDialog, this);
                    m_applicationPtr->addModalDialog(ModalDialogFactory::createEnablingEffectPluginConfirmDialog(
                        m_applicationPtr, fileURI, handle, type, callback));
                    succeeded = needsConfirmDialog = true;
                }
            }
            if (!needsConfirmDialog) {
                loadModel(fileURI, handle, type, project, error);
                succeeded = !error.hasReason();
            }
            break;
        }
        case kDialogTypeLoadModelFile: {
            loadModel(fileURI, handle, type, project, error);
            succeeded = !error.hasReason();
            break;
        }
        case kDialogTypeOpenAudioFile: {
            succeeded = loadAudioFile(fileURI, project, error);
            if (succeeded) {
                project->seek(0, true);
                project->restart();
            }
            break;
        }
        case kDialogTypeOpenVideoFile: {
            succeeded = loadVideoFile(fileURI, project, error);
            break;
        }
        case kDialogTypeOpenCameraMotionFile: {
            succeeded = loadCameraMotion(fileURI, project, error);
            break;
        }
        case kDialogTypeOpenLightMotionFile: {
            succeeded = loadLightMotion(fileURI, project, error);
            break;
        }
        case kDialogTypeOpenModelMotionFile: {
            succeeded = loadModelMotion(fileURI, project, error);
            break;
        }
        case kDialogTypeOpenEffectFile: {
            if (project->isEffectPluginEnabled()) {
                succeeded = loadEffectSource(fileURI, project, error);
            }
            else {
                const ModalDialogFactory::EnablingEffectPluginDialogCallback callback(
                    loadEffectAfterEnablingEffectPluginDialog, this);
                m_applicationPtr->addModalDialog(ModalDialogFactory::createEnablingEffectPluginConfirmDialog(
                    m_applicationPtr, fileURI, handle, type, callback));
                succeeded = true;
            }
            break;
        }
        case kDialogTypeUserCallback: {
            if (m_queryFileDialogCallbacks.m_accept) {
                m_queryFileDialogCallbacks.m_accept(fileURI, project, error, m_queryFileDialogCallbacks.m_opaque);
            }
            resetTransientQueryFileDialogCallback();
            break;
        }
        case kDialogTypeMaxEnum:
        default:
            break;
        }
    }
    return succeeded;
}

bool
DefaultFileManager::saveAsFile(const URI &fileURI, DialogType type, Project *project, Error &error)
{
    nanoem_parameter_assert(project, "must NOT be nullptr");
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    bool succeeded = false;
    switch (type) {
    case kDialogTypeSaveCameraMotionFile: {
        succeeded = saveCameraMotion(fileURI, project, error);
        break;
    }
    case kDialogTypeSaveLightMotionFile: {
        succeeded = saveLightMotion(fileURI, project, error);
        break;
    }
    case kDialogTypeSaveModelMotionFile: {
        succeeded = saveModelMotion(fileURI, project, error);
        break;
    }
    case kDialogTypeSaveModelFile: {
        succeeded = saveModel(fileURI, project, error);
        break;
    }
    case kDialogTypeSaveProjectFile: {
        succeeded = saveProject(fileURI, project, error);
        break;
    }
    case kDialogTypeUserCallback: {
        if (m_queryFileDialogCallbacks.m_accept) {
            m_queryFileDialogCallbacks.m_accept(fileURI, project, error, m_queryFileDialogCallbacks.m_opaque);
        }
        resetTransientQueryFileDialogCallback();
        break;
    }
    default:
        break;
    }
    return succeeded;
}

IModalDialog *
DefaultFileManager::handleAddingModelDialog(void *userData, Model *model, Project *project)
{
    BX_UNUSED_1(project);
    DefaultFileManager *fileManager = static_cast<DefaultFileManager *>(userData);
    Progress progress(project, 0);
    Error error;
    fileManager->loadModelEffectSettingFile(model, project, progress, error);
    IModalDialog *dialog = nullptr;
    if (!project->loadAttachedDrawableEffect(model, progress, error)) {
        BaseApplicationService *applicationPtr = fileManager->m_applicationPtr;
        dialog = error.createModalDialog(applicationPtr);
        applicationPtr->eventPublisher()->publishErrorEvent(error);
    }
    return dialog;
}

IModalDialog *
DefaultFileManager::loadEffectAfterEnablingEffectPluginDialog(
    void *userData, const URI &fileURI, nanoem_u16_t /* handle */, int /* type */, Project *project)
{
    DefaultFileManager *fileManager = static_cast<DefaultFileManager *>(userData);
    IModalDialog *dialog = nullptr;
    Error error;
    if (!fileManager->loadEffectSource(fileURI, project, error) && error.hasReason()) {
        BaseApplicationService *applicationPtr = fileManager->m_applicationPtr;
        dialog = error.createModalDialog(applicationPtr);
        applicationPtr->eventPublisher()->publishErrorEvent(error);
    }
    return dialog;
}

IModalDialog *
DefaultFileManager::loadModelAfterEnablingEffectPluginDialog(
    void *userData, const URI &fileURI, nanoem_u16_t handle, int type, Project *project)
{
    DefaultFileManager *fileManager = static_cast<DefaultFileManager *>(userData);
    Error error;
    IModalDialog *dialog = nullptr;
    fileManager->loadModel(fileURI, handle, static_cast<DialogType>(type), project, error);
    if (error.hasReason()) {
        BaseApplicationService *applicationPtr = fileManager->m_applicationPtr;
        dialog = error.createModalDialog(applicationPtr);
        applicationPtr->eventPublisher()->publishErrorEvent(error);
    }
    return dialog;
}

bool
DefaultFileManager::internalLoadEffectSourceFile(
    LoadEffectCallback callback, const URI &fileURI, Project *project, Error &error)
{
    URI sourceURI;
    Progress progress(project, 0);
    Effect *effect = project->findEffect(fileURI);
    bool cancelled = false;
    if (effect) {
        callback(effect, project, error);
    }
    else {
        effect = project->createEffect();
        if (project->loadEffectFromSource(fileURI, effect, sourceURI, progress, error)) {
            effect->setFileURI(sourceURI);
            callback(effect, project, error);
        }
        else {
            cancelled = error.isCancelled();
        }
        if (cancelled || error.hasReason()) {
            project->destroyEffect(effect);
        }
    }
    return !error.hasReason();
}

bool
DefaultFileManager::loadPlainText(const URI &fileURI, Project *project, Error &error)
{
    FileReaderScope scope(&m_translator);
    bool succeeded = false;
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        if (!error.hasReason()) {
            String message;
            nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
            StringUtils::getUtf8String(
                reinterpret_cast<const char *>(bytes.data()), bytes.size(), NANOEM_CODEC_TYPE_SJIS, factory, message);
            m_applicationPtr->addModalDialog(ModalDialogFactory::createDisplayPlainTextDialog(
                m_applicationPtr, fileURI.lastPathComponent(), message));
            succeeded = !error.hasReason();
        }
    }
    return succeeded;
}

bool
DefaultFileManager::loadModel(
    const URI &fileURI, nanoem_u16_t /* handle */, DialogType type, Project *project, Error &error)
{
    bool succeeded = false;
    if (fileURI.pathExtension() == String("zip")) {
        FileReaderScope scope(&m_translator);
        if (scope.open(fileURI, error)) {
            ByteArray bytes;
            FileUtils::read(scope, bytes, error);
            if (!error.hasReason()) {
                succeeded = loadModelFromArchive(scope.release(), fileURI, type, project, error);
            }
        }
    }
    else if (Model::isLoadableExtension(fileURI)) {
        succeeded = m_applicationPtr->hasModalDialog() ? true : internalLoadModel(fileURI, type, project, error);
    }
    else if (Accessory::isLoadableExtension(fileURI)) {
        succeeded = internalLoadAccessory(fileURI, project, error);
    }
    else if (fileURI.pathExtension() == String("vac")) {
        VACData data(project->unicodeStringFactory());
        if (data.read(fileURI, error)) {
            String newAccessoryPath(fileURI.absolutePathByDeletingLastPathComponent());
            newAccessoryPath.append("/");
            newAccessoryPath.append(data.m_filename.c_str());
            Accessory *accessory = project->createAccessory();
            Progress progress(project, 0);
            if (internalLoadAccessoryFromFile(URI::createFromFilePath(newAccessoryPath), accessory, progress, error)) {
                project->loadAttachedDrawableEffect(accessory, progress, error);
                succeeded = !error.isCancelled() && !error.hasReason();
                if (succeeded) {
                    project->addAccessory(accessory);
                    project->setActiveAccessory(accessory);
                }
            }
            if (!succeeded) {
                project->destroyAccessory(accessory);
                accessory = nullptr;
            }
            if (accessory) {
                accessory->setName(data.m_name);
                accessory->setTranslation(data.m_translation);
                accessory->setOrientation(data.m_orientation);
                accessory->setScaleFactor(data.m_scaleFactor);
            }
        }
    }
    else {
        String reason("Unsupported loading model type: ");
        reason.append(fileURI.absolutePathConstString());
        error = Error(reason.c_str(), nullptr, Error::kDomainTypeApplication);
    }
    return succeeded;
}

bool
DefaultFileManager::loadCameraMotion(const URI &fileURI, Project *project, Error &error)
{
    Motion *motion = project->createMotion();
    bool succeeded = false;
    if (loadMotion(fileURI, motion, project->currentLocalFrameIndex(), error)) {
        if (isCameraLightMotion(motion)) {
            motion->writeLoadCameraCommandMessage(fileURI, error);
            succeeded = !error.hasReason();
        }
        else {
            error = Error(translator()->translate("nanoem.error.motion.not-camera-and-light.reason"), "",
                Error::kDomainTypeApplication);
        }
    }
    if (succeeded) {
        project->setCameraMotion(motion);
    }
    else {
        project->destroyMotion(motion);
    }
    return succeeded;
}

bool
DefaultFileManager::loadLightMotion(const URI &fileURI, Project *project, Error &error)
{
    Motion *motion = project->createMotion();
    bool succeeded = false;
    if (loadMotion(fileURI, motion, project->currentLocalFrameIndex(), error)) {
        if (isCameraLightMotion(motion)) {
            motion->writeLoadLightCommandMessage(fileURI, error);
            succeeded = !error.hasReason();
        }
        else {
            error = Error(translator()->translate("nanoem.error.motion.not-camera-and-light.reason"), "",
                Error::kDomainTypeApplication);
        }
    }
    if (succeeded) {
        project->setLightMotion(motion);
    }
    else {
        project->destroyMotion(motion);
    }
    return succeeded;
}

bool
DefaultFileManager::loadModelMotion(const URI &fileURI, Project *project, Error &error)
{
    bool succeeded = false;
    if (Model *model = project->activeModel()) {
        if (Motion::isLoadableExtension(fileURI)) {
            Motion *motion = project->createMotion(), *lastMotionPtr = motion;
            if (loadMotion(fileURI, motion, project->currentLocalFrameIndex(), error)) {
                if (!isCameraLightMotion(motion)) {
                    motion->writeLoadModelCommandMessage(model->handle(), fileURI, error);
                    succeeded = !error.hasReason();
                }
                else {
                    error = Error(translator()->translate("nanoem.error.motion.not-model.reason"), "",
                        Error::kDomainTypeApplication);
                }
            }
            if (succeeded) {
                StringSet bones, morphs;
                if (!motion->testAllMissingModelObjects(model, bones, morphs)) {
                    String message, name;
                    StringUtils::getUtf8String(
                        nanoemMotionGetTargetModelName(motion->data()), project->unicodeStringFactory(), name);
                    StringUtils::format(
                        message, translator()->translate("nanoem.motion.model.diagnostics.message"), name.c_str());
                    message.append("\n");
                    if (!bones.empty()) {
                        StringUtils::format(message, "\n%s\n",
                            translator()->translate("nanoem.motion.model.diagnostics.all-missing-bones"));
                        for (StringSet::const_iterator it = bones.begin(), end = bones.end(); it != end; ++it) {
                            message.append("* ");
                            message.append(it->c_str());
                            message.append("\n");
                        }
                    }
                    if (!morphs.empty()) {
                        StringUtils::format(message, "\n%s\n",
                            translator()->translate("nanoem.motion.model.diagnostics.all-missing-morphs"));
                        for (StringSet::const_iterator it = morphs.begin(), end = morphs.end(); it != end; ++it) {
                            message.append("* ");
                            message.append(it->c_str());
                            message.append("\n");
                        }
                    }
                    m_applicationPtr->addModalDialog(ModalDialogFactory::createDisplayPlainTextDialog(
                        m_applicationPtr, translator()->translate("nanoem.motion.model.diagnostics.title"), message));
                }
                motion->selectAllModelObjectKeyframes(model);
                lastMotionPtr = project->addModelMotion(motion, model);
                project->restart();
            }
            project->destroyMotion(lastMotionPtr);
        }
        else if (model::BindPose::isLoadableExtension(fileURI)) {
            FileReaderScope scope(&m_translator);
            if (scope.open(fileURI, error)) {
                ByteArray bytes;
                FileUtils::read(scope, bytes, error);
                if (!error.hasReason()) {
                    succeeded = model->loadPose(bytes, error);
                }
            }
        }
    }
    else {
        error = Error(translator()->translate("nanoem.error.motion.no-active-model.reason"),
            translator()->translate("nanoem.error.motion.no-active-model.recovery-suggestion"),
            Error::kDomainTypeApplication);
    }
    return succeeded;
}

bool
DefaultFileManager::loadMotion(const URI &fileURI, Motion *motion, nanoem_frame_index_t offset, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    nanoem_parameter_assert(motion, "must not be nullptr");
    FileReaderScope scope(&m_translator);
    bool succeeded = false;
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        if (!error.hasReason()) {
            motion->setFormat(fileURI);
            succeeded = motion->load(bytes, offset, error);
            if (succeeded) {
                motion->setFileURI(fileURI);
            }
        }
    }
    return succeeded;
}

bool
DefaultFileManager::loadEffectSource(const URI &fileURI, Project *project, Error &error)
{
    return internalLoadEffectSourceFile(setEffectCallback, fileURI, project, error);
}

bool
DefaultFileManager::internalLoadAccessory(const URI &fileURI, Project *project, Error &error)
{
    Accessory *accessory = project->createAccessory();
    Progress progress(project, 0);
    bool succeeded = false;
    if (internalLoadAccessoryFromFile(fileURI, accessory, progress, error)) {
        project->loadAttachedDrawableEffect(accessory, progress, error);
        succeeded = !error.isCancelled() && !error.hasReason();
        if (succeeded) {
            accessory->setName(fileURI.lastPathComponent());
            project->addAccessory(accessory);
            project->setActiveAccessory(accessory);
        }
    }
    if (!succeeded) {
        project->removeAccessory(accessory);
        project->destroyAccessory(accessory);
    }
    progress.complete();
    return succeeded;
}

bool
DefaultFileManager::internalLoadModel(const URI &fileURI, DialogType type, Project *project, Error &error)
{
    Model *model = project->createModel();
    bool succeeded = false;
    if (internalLoadModelFromFile(fileURI, model, error)) {
        const ModalDialogFactory::AddingModelDialogCallback callback(handleAddingModelDialog, this);
        if (type == kDialogTypeLoadModelFile) {
            IModalDialog *dialog =
                ModalDialogFactory::createLoadingModelConfirmDialog(m_applicationPtr, model, callback);
            dialog->onAccepted(project);
            succeeded = !dialog->isCancelled();
            nanoem_delete(dialog);
        }
        else {
            m_applicationPtr->addModalDialog(
                ModalDialogFactory::createLoadingModelConfirmDialog(m_applicationPtr, model, callback));
            /* force adding confirm saving project dialog */
            project->globalCamera()->setDirty(true);
            succeeded = true;
        }
    }
    if (!succeeded) {
        project->removeModel(model);
        project->destroyModel(model);
    }
    return succeeded;
}

bool
DefaultFileManager::internalLoadAccessoryFromFile(const URI &fileURI, Accessory *accessory, Progress &progress, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    nanoem_parameter_assert(accessory, "must not be nullptr");
    FileReaderScope scope(&m_translator);
    bool succeeded = false;
    progress.tryLoadingItem(fileURI);
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        if (!error.hasReason() && accessory->load(bytes, error)) {
            accessory->setFileURI(fileURI);
            accessory->upload();
            accessory->loadAllImages(progress, error);
            succeeded = !error.isCancelled();
            if (succeeded) {
                accessory->writeLoadCommandMessage(error);
            }
        }
    }
    return succeeded;
}

bool
DefaultFileManager::internalLoadModelFromFile(const URI &fileURI, Model *model, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    nanoem_parameter_assert(model, "must NOT be nullptr");
    FileReaderScope scope(&m_translator);
    bool succeeded = false;
    if (scope.open(fileURI, error)) {
        ByteArray bytes;
        FileUtils::read(scope, bytes, error);
        if (!error.hasReason() && model->load(bytes, error)) {
            model->setFileURI(fileURI);
            succeeded = true;
        }
    }
    return succeeded;
}

bool
DefaultFileManager::loadModelFromArchive(
    IFileReader *reader, const URI &fileURI, DialogType type, Project *project, Error &error)
{
    nanoem_parameter_assert(!fileURI.isEmpty(), "must NOT be empty");
    bool canClose = true, canDestroyFileReader = true;
    const String &entryPointPath = fileURI.fragment();
    const String &extension = URI::pathExtension(entryPointPath);
    if (Model::isLoadableExtension(extension)) {
        Model *model = project->createModel();
        model->setFileURI(fileURI);
        if (model->loadArchive(entryPointPath, reader, error)) {
            const ModalDialogFactory::DestroyReaderCallback callback(FileUtils::destroyFileReader, reader);
            if (type == kDialogTypeLoadModelFile) {
                IModalDialog *dialog =
                    ModalDialogFactory::createLoadingArchivedModelConfirmDialog(m_applicationPtr, model, callback);
                dialog->onAccepted(project);
                nanoem_delete(dialog);
            }
            else {
                m_applicationPtr->addModalDialog(
                    ModalDialogFactory::createLoadingArchivedModelConfirmDialog(m_applicationPtr, model, callback));
                canClose = false;
            }
            canDestroyFileReader = false;
        }
        else {
            project->removeModel(model);
            project->destroyModel(model);
        }
    }
    else if (Accessory::isLoadableExtension(extension)) {
        Progress progress(project, 0);
        Accessory *accessory = project->createAccessory();
        if (accessory->uploadArchive(entryPointPath, reader, progress, error)) {
            accessory->setVisible(true);
            accessory->setName(URI::stringByDeletingPathExtension(URI::lastPathComponent(fileURI.fragment())));
            accessory->setFileURI(fileURI);
            project->addAccessory(accessory);
            project->setActiveAccessory(accessory);
        }
        else {
            project->destroyAccessory(accessory);
        }
        progress.complete();
    }
    else if (extension == String("txt")) {
        Archiver archiver(reader);
        if (archiver.open(error)) {
            Archiver::Entry entry;
            if (archiver.findEntry(entryPointPath, entry, error)) {
                ByteArray bytes;
                if (archiver.extract(entry, bytes, error)) {
                    String message;
                    nanoem_unicode_string_factory_t *factory = project->unicodeStringFactory();
                    StringUtils::getUtf8String(reinterpret_cast<const char *>(bytes.data()), bytes.size(),
                        NANOEM_CODEC_TYPE_SJIS, factory, message);
                    const char *p = strrchr(entryPointPath.c_str(), '/');
                    m_applicationPtr->addModalDialog(ModalDialogFactory::createDisplayPlainTextDialog(
                        m_applicationPtr, p ? p + 1 : entryPointPath, message));
                }
            }
            archiver.close(error);
        }
    }
    if (canDestroyFileReader) {
        FileUtils::destroyFileReader(reader);
    }
    return canClose;
}

bool
DefaultFileManager::saveProject(const URI &fileURI, Project *project, Error &error)
{
    FileWriterScope writeScope;
    bool succeeded = false;
    if (writeScope.open(fileURI, error)) {
        const URI &lastFileURI = project->fileURI();
        const String &lastFileExtension = lastFileURI.pathExtension(), &savingFileExtension = fileURI.pathExtension();
        if (!lastFileExtension.empty() &&
            !StringUtils::equals(lastFileExtension.c_str(), Project::kFileSystemBasedNativeFormatFileExtension) &&
            !StringUtils::equals(lastFileExtension.c_str(), Project::kPolygonMovieMakerFileExtension) &&
            StringUtils::equals(savingFileExtension.c_str(), Project::kFileSystemBasedNativeFormatFileExtension)) {
            error = Error(m_translator.translate("nanoem.error.convert-to-nmm.reason"),
                m_translator.translate("nanoem.error.convert-to-nmm.recovery-suggestion"),
                Error::kDomainTypeApplication);
        }
        else {
            nanoem_u8_t expectedChecksum[SHA256_BLOCK_SIZE];
            IFileWriter *writer = writeScope.writer();
            project->setFileURI(fileURI);
            if ((StringUtils::equals(savingFileExtension.c_str(), Project::kArchivedNativeFormatFileExtension) &&
                    project->saveAsArchive(writeScope.writer(), error))) {
                writeScope.commit(error);
                succeeded = !error.hasReason();
            }
            else if ((StringUtils::equals(
                          savingFileExtension.c_str(), Project::kFileSystemBasedNativeFormatFileExtension) &&
                         project->saveAsBinary(writer, Project::kBinaryFormatNative, expectedChecksum, error)) ||
                (StringUtils::equals(savingFileExtension.c_str(), Project::kPolygonMovieMakerFileExtension) &&
                    project->saveAsBinary(writer, Project::kBinaryFormatPMM, expectedChecksum, error))) {
                const bool validate = false;
                succeeded = writer->close(error) &&
                    (validate ? validateWrittenFile(writeScope, expectedChecksum, error) : writer->commit(error));
            }
            else {
                project->setFileURI(lastFileURI);
                writeScope.rollback(error);
            }
            if (succeeded) {
                project->writeRedoMessage();
            }
        }
    }
    FileUtils::TransientPath transientPath;
    bool created = FileUtils::createTransientFile(fileURI.absolutePath(), transientPath);
    if (project->hasTransientPath()) {
        FileUtils::TransientPath lastTransientPath(project->transientPath());
        FileUtils::deleteTransientFile(lastTransientPath);
        project->setTransientPath(transientPath);
    }
    if (created) {
        project->setTransientPath(transientPath);
    }
    return succeeded;
}

bool
DefaultFileManager::saveModel(const URI &fileURI, Project *project, Error &error)
{
    Model *model = project->activeModel();
    bool succeeded = false;
    if (model && Model::isLoadableExtension(fileURI)) {
        FileWriterScope scope;
        if (scope.open(fileURI, error)) {
            succeeded = model->save(scope.writer(), error);
        }
        if (succeeded) {
            scope.commit(error);
            succeeded = !error.hasReason();
        }
        else {
            scope.rollback(error);
        }
    }
    return succeeded;
}

bool
DefaultFileManager::saveCameraMotion(const URI &fileURI, Project *project, Error &error)
{
    FileWriterScope scope;
    bool succeeded = false;
    if (Motion::isLoadableExtension(fileURI) && scope.open(fileURI, error)) {
        Motion *motion = project->cameraMotion();
        motion->setFormat(fileURI);
        succeeded = motion->save(scope.writer(), nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, error);
        if (succeeded) {
            motion->setFileURI(fileURI);
            scope.commit(error);
            succeeded = !error.hasReason();
        }
        else {
            scope.rollback(error);
        }
    }
    return succeeded;
}

bool
DefaultFileManager::saveLightMotion(const URI &fileURI, Project *project, Error &error)
{
    FileWriterScope scope;
    bool succeeded = false;
    if (Motion::isLoadableExtension(fileURI) && scope.open(fileURI, error)) {
        Motion *motion = project->lightMotion();
        motion->setFormat(fileURI);
        succeeded = motion->save(scope.writer(), nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, error);
        if (succeeded) {
            motion->setFileURI(fileURI);
            scope.commit(error);
            succeeded = !error.hasReason();
        }
        else {
            scope.rollback(error);
        }
    }
    return succeeded;
}

bool
DefaultFileManager::saveModelMotion(const URI &fileURI, Project *project, Error &error)
{
    FileWriterScope scope;
    bool succeeded = false;
    if (Model *model = project->activeModel()) {
        if (Motion::isLoadableExtension(fileURI)) {
            Motion *motion = project->resolveMotion(model);
            if (motion && scope.open(fileURI, error)) {
                motion->setFormat(fileURI);
                succeeded = motion->save(scope.writer(), model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, error);
                if (succeeded) {
                    motion->setFileURI(fileURI);
                    scope.commit(error);
                    succeeded = !error.hasReason();
                }
                else {
                    scope.rollback(error);
                }
            }
        }
        else if (model::BindPose::isLoadableExtension(fileURI) && scope.open(fileURI, error)) {
            succeeded = model->savePose(scope.writer(), error);
            if (succeeded) {
                scope.commit(error);
                succeeded = !error.hasReason();
            }
            else {
                scope.rollback(error);
            }
        }
    }
    return succeeded;
}

bool
DefaultFileManager::validateWrittenFile(FileWriterScope &scope, const nanoem_u8_t *expectedChecksum, Error &error)
{
    nanoem_u8_t actualChecksum[SHA256_BLOCK_SIZE];
    IFileWriter *writer = scope.writer();
    FileReaderScope readerScope(&m_translator);
    ByteArray bytes;
    bool succeeded = false;
    if (readerScope.open(writer->writeDestinationURI(), error)) {
        FileUtils::read(readerScope, bytes, error);
        readerScope.reader()->close(error);
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, bytes.data(), bytes.size());
        sha256_final(&ctx, actualChecksum);
        if (memcmp(expectedChecksum, actualChecksum, sizeof(actualChecksum)) == 0 && !error.hasReason()) {
            scope.commit(error);
            succeeded = !error.hasReason();
        }
        else {
            error = Error(m_translator.translate("nanoem.error.project.save.reason"),
                m_translator.translate("nanoem.error.project.save.recovery-suggestion"), Error::kDomainTypeApplication);
        }
    }
    if (!succeeded) {
        scope.rollback(error);
    }
    return succeeded;
}

bool
DefaultFileManager::decodeAudioWave(
    const Project *project, PluginFactory::DecoderPluginProxy &proxy, ByteArray &bytes, Error &error)
{
    ByteArray chunk;
    bool succeeded = true;
    for (nanoem_frame_index_t i = 0, duration = project->duration(); succeeded && i < duration; i++) {
        succeeded &= proxy.decodeAudioFrame(i, chunk, error);
        if (!chunk.empty()) {
            bytes.insert(bytes.end(), chunk.begin(), chunk.end());
        }
    }
    return succeeded;
}

bool
DefaultFileManager::isCameraLightMotion(const Motion *motion)
{
    nanoem_unicode_string_factory_t *factory = motion->project()->unicodeStringFactory();
    const nanoem_unicode_string_t *name = nanoemMotionGetTargetModelName(motion->data());
    String targetModelName;
    StringUtils::getUtf8String(name, factory, targetModelName);
    return StringUtils::equals(
        reinterpret_cast<const char *>(Motion::kCameraAndLightTargetModelName), targetModelName.c_str());
}

} /* namespace nanoem */
