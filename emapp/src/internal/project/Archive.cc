/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
*/

#include "emapp/internal/project/Archive.h"

#include "emapp/Accessory.h"
#include "emapp/BaseAudioPlayer.h"
#include "emapp/Effect.h"
#include "emapp/FileUtils.h"
#include "emapp/Grid.h"
#include "emapp/IAudioPlayer.h"
#include "emapp/IBackgroundVideoRenderer.h"
#include "emapp/ICamera.h"
#include "emapp/IFileManager.h"
#include "emapp/ILight.h"
#include "emapp/ListUtils.h"
#include "emapp/Model.h"
#include "emapp/PhysicsEngine.h"
#include "emapp/PluginFactory.h"
#include "emapp/Progress.h"
#include "emapp/Project.h"
#include "emapp/ShadowCamera.h"
#include "emapp/StringUtils.h"
#include "emapp/UUID.h"
#include "emapp/internal/project/Native.h"
#include "emapp/private/CommonInclude.h"

#include "../../protoc/project.pb-c.h"
#include "bx/rng.h"

namespace nanoem {
namespace internal {
namespace project {
namespace {

class PrivateArchiveUtils NANOEM_DECL_SEALED : private NonCopyable {
public:
    static inline const char *trimMovingDirectoryPath(const char *path) NANOEM_DECL_NOEXCEPT;
    static StringMap addUUID(const Project *project, const void *ptr, const StringMap &values);
};

const char *
PrivateArchiveUtils::trimMovingDirectoryPath(const char *path) NANOEM_DECL_NOEXCEPT
{
    while (StringUtils::equals(path, "../", 3)) {
        path += 3;
    }
    return path;
}

StringMap
PrivateArchiveUtils::addUUID(const Project *project, const void *ptr, const StringMap &values)
{
    StringMap::const_iterator it = values.find("uuid");
    StringMap newValeus(values);
    if (it == values.end()) {
        newValeus.insert(tinystl::make_pair(String("uuid"), project->generateUUID(ptr).toString()));
    }
    return newValeus;
}

} /* namespace anonymous */

const char *const Archive::kManifestEntryPath = "manifest.nmm";
const char *const Archive::kManifestCompatEntryPath = "project.pb";
const char *const Archive::kBGMEntryPath = "Wav/BGM.wav";

Archive::Archive(Project *project, const URI &fileURI)
    : m_fileURI(fileURI)
    , m_project(project)
    , m_archiver(nullptr)
    , m_progress(nullptr)
{
}

Archive::~Archive() NANOEM_DECL_NOEXCEPT
{
    m_project = nullptr;
    nanoem_delete_safe(m_archiver);
    nanoem_delete_safe(m_progress);
}

bool
Archive::load(ISeekableReader *reader, Error &error)
{
    SG_PUSH_GROUPF("internal::project::Archive::load(fileURI=%s)", m_fileURI.absolutePathConstString());
    m_archiver = nanoem_new(Archiver(reader));
    bool succeeded = m_archiver->open(error);
    if (succeeded) {
        const Archiver::EntryList &entries = m_archiver->allEntries(error);
        Archiver::EntryList motionList, modelList, accessoryList;
        Archiver::Entry projectEntry;
        for (Archiver::EntryList::const_iterator it = entries.begin(), end = entries.end(); it != end; ++it) {
            const Archiver::Entry &entry = *it;
            if (const char *extension = entry.extensionPtr()) {
                if (Accessory::isLoadableExtension(extension)) {
                    accessoryList.push_back(entry);
                }
                else if (Model::isLoadableExtension(extension)) {
                    modelList.push_back(entry);
                }
                else if (Motion::isLoadableExtension(extension)) {
                    motionList.push_back(entry);
                }
                else if (StringUtils::equals(entry.filenamePtr(), kManifestEntryPath) ||
                    StringUtils::equals(entry.filenamePtr(), kManifestCompatEntryPath)) {
                    projectEntry = entry;
                }
            }
        }
        static const nanoem_u32_t kAdditionalProgressLoadingItems = 2;
        m_progress = nanoem_new(Progress(m_project,
            Inline::saturateInt32U(
                accessoryList.size() + modelList.size() + motionList.size() + kAdditionalProgressLoadingItems)));
        ByteArray bytes;
        Archiver::Entry foundEntry;
        succeeded &= m_archiver->findEntry(projectEntry.m_path, foundEntry, error) &&
            m_archiver->extract(foundEntry, bytes, error) && loadAllAccessories(accessoryList, error) &&
            loadAllModels(modelList, error) && loadAllMotions(motionList, error);
        if (succeeded) {
            Native native(m_project);
            native.load(bytes.data(), bytes.size(), Native::kFileTypeArchive, error, nullptr);
            succeeded &= loadAllEffects(native, error);
            succeeded &= loadAudio(native, error);
            succeeded &= loadVideo(native, error);
            if (succeeded) {
                ICamera *camera = m_project->activeCamera();
                const Vector3 angle(camera->angle()), lookAt(camera->lookAt());
                const nanoem_f32_t fov = camera->fovRadians(), distance = camera->distance();
                const bool perspective(camera->isPerspective());
                ILight *light = m_project->activeLight();
                const Vector3 color(light->color()), direction(light->direction());
                m_project->setPlayingSegment(TimelineSegment(m_project));
                m_project->setSelectionSegment(TimelineSegment(m_project));
                const nanoem_frame_index_t currentLocalFrameIndex = m_project->currentLocalFrameIndex();
                m_project->seek(currentLocalFrameIndex, true);
                m_project->restart(currentLocalFrameIndex);
                m_project->update();
                camera->setAngle(angle);
                camera->setLookAt(lookAt);
                camera->setFovRadians(fov);
                camera->setDistance(distance);
                camera->setPerspective(perspective);
                camera->setDirty(false);
                light->setColor(color);
                light->setDirection(direction);
                light->setDirty(false);
            }
            m_progress->complete();
        }
        succeeded &= m_archiver->close(error);
        nanoem_delete_safe(m_progress);
    }
    nanoem_delete_safe(m_archiver);
    SG_POP_GROUP();
    return succeeded;
}

bool
Archive::save(ISeekableWriter *writer, Error &error)
{
    nanoem_assert(!m_fileURI.isEmpty(), "fileURI must NOT be empty");
    m_archiver = nanoem_new(Archiver(writer));
    bool succeeded = m_archiver->open(error);
    if (succeeded) {
        StringSet reservedNameSet;
        Native native(m_project);
        succeeded &= saveAllModels(native, reservedNameSet, error) &&
            saveAllAccessories(native, reservedNameSet, error) && saveAllMotions(error) && saveAudio(error) &&
            saveVideo(error);
        if (succeeded) {
            Archiver::Entry entry;
            ByteArray bytes;
            entry.m_path = kManifestEntryPath;
            native.setAnnotation("generator.name", "nanoem");
            native.setAnnotation("generator.version", nanoemGetVersionString());
            native.setAnnotation("generator.archive.filename", m_fileURI.lastPathComponent());
            native.save(bytes, Native::kFileTypeArchive, error);
            succeeded &= m_archiver->addEntry(entry, bytes, error);
        }
        m_accessoryMotionSet.clear();
        m_modelMotionSet.clear();
        succeeded &= m_archiver->close(error);
    }
    return succeeded;
}

bool
Archive::loadAudio(Native &native, Error &error)
{
    bool continuable = true;
    const URI &fileURI = native.audioURI();
    Archiver::Entry foundEntry;
    ByteArray bytes;
    if (!m_progress->tryLoadingItem(fileURI)) {
        error = Error::cancelled();
        continuable = false;
    }
    else if (m_archiver->findEntry(fileURI.fragment(), foundEntry, error) &&
        m_archiver->extract(foundEntry, bytes, error)) {
        IAudioPlayer *audio = m_project->audioPlayer();
        continuable = audio->load(bytes, error);
        if (continuable) {
            m_project->setBaseDuration(audio);
        }
        continuable = !m_project->isCancelRequested();
    }
    m_progress->increment();
    return continuable;
}

bool
Archive::loadVideo(Native &native, Error &error)
{
    bool continuable = true;
    const URI &fileURI = native.videoURI();
    Archiver::Entry foundEntry;
    if (!m_progress->tryLoadingItem(fileURI)) {
        error = Error::cancelled();
        continuable = false;
    }
    else if (m_archiver->findEntry(fileURI.fragment(), foundEntry, error)) {
        FileWriterScope scope;
        const JSON_Object *config = json_object(m_project->applicationConfiguration());
        if (const char *tempPath = json_object_dotget_string(config, "project.tmp.path")) {
            String absoluteTempPath(tempPath);
            absoluteTempPath.append("/");
            absoluteTempPath.append(URI::lastPathComponent(fileURI.fragment()).c_str());
            const URI &tempURI = URI::createFromFilePath(absoluteTempPath);
            continuable = scope.open(tempURI, error);
            if (continuable && m_archiver->extract(foundEntry, scope.writer(), error)) {
                scope.commit(error);
                continuable = m_project->backgroundVideoRenderer()->load(tempURI, error);
            }
        }
        continuable = !m_project->isCancelRequested();
    }
    m_progress->increment();
    return continuable;
}

bool
Archive::loadAllAccessories(const Archiver::EntryList &accessoryList, Error &error)
{
    bool continuable = true;
    for (Archiver::EntryList::const_iterator it = accessoryList.begin(), end = accessoryList.end();
         it != end && continuable; ++it) {
        const Archiver::Entry &entry = *it;
        continuable &= loadAccessory(entry.m_path, error);
    }
    return continuable;
}

bool
Archive::loadAccessory(const String &entryPath, Error &error)
{
    const URI &fileURI = URI::createFromFilePath(m_fileURI.absolutePath(), entryPath);
    Accessory *accessory = m_project->createAccessory();
    bool continuable = true;
    accessory->setFileURI(fileURI);
    if (!m_progress->tryLoadingItem(fileURI)) {
        error = Error::cancelled();
        continuable = false;
    }
    if (accessory->uploadArchive(entryPath, *m_archiver, *m_progress, error)) {
        const String &name = URI::lastPathComponent(URI::stringByDeletingLastPathComponent(entryPath));
        IDrawable *drawable = accessory;
        accessory->setName(name);
        m_project->addAccessory(accessory);
        m_pathDrawables.insert(tinystl::make_pair(drawable, entryPath));
        m_progress->increment();
    }
    else {
        m_project->destroyAccessory(accessory);
        continuable = false;
    }
    return continuable;
}

bool
Archive::loadAllModels(const Archiver::EntryList &modelList, Error &error)
{
    bool continuable = true;
    for (Archiver::EntryList::const_iterator it = modelList.begin(), end = modelList.end(); it != end && continuable;
         ++it) {
        const Archiver::Entry &entry = *it;
        continuable &= loadModel(entry.m_path, error);
    }
    return continuable;
}

bool
Archive::loadModel(const String &entryPath, Error &error)
{
    const URI &fileURI = URI::createFromFilePath(m_fileURI.absolutePath(), entryPath);
    Model *model = m_project->createModel();
    bool continuable = true, added = false;
    model->setFileURI(fileURI);
    if (!m_progress->tryLoadingItem(fileURI)) {
        error = Error::cancelled();
        continuable = false;
    }
    else if (model->loadArchive(entryPath, *m_archiver, error)) {
        model->setupAllBindings();
        model->createAllImages();
        model->upload();
        model->uploadArchive(*m_archiver, *m_progress, error);
        model->setDirty(false);
        if (!error.hasReason()) {
            IDrawable *drawable = model;
            m_project->addModel(model);
            m_pathDrawables.insert(tinystl::make_pair(drawable, entryPath));
            added = true;
        }
    }
    continuable &= added;
    if (added) {
        m_progress->increment();
    }
    else {
        m_project->destroyModel(model);
    }
    return continuable;
}

bool
Archive::loadAllMotions(const Archiver::EntryList &motionList, Error &error)
{
    bool continuable = true;
    for (Archiver::EntryList::const_iterator it = motionList.begin(), end = motionList.end(); it != end && continuable;
         ++it) {
        const Archiver::Entry &entry = *it;
        continuable &= loadMotion(entry, error);
    }
    return continuable;
}

bool
Archive::loadMotion(const Archiver::Entry &entry, Error &error)
{
    Motion *motion = m_project->createMotion(), *lastMotion = nullptr;
    bool continuable = true;
    if (const char *extension = entry.extensionPtr()) {
        const char *filename = entry.filenamePtr();
        const String name(filename, size_t(extension - filename - 1));
        motion->setFormat(extension);
        Archiver::Entry motionEntry;
        ByteArray bytes;
        if (!m_progress->tryLoadingItem(filename)) {
            error = Error::cancelled();
            continuable = false;
        }
        else {
            continuable &= m_archiver->findEntry(entry.m_path, motionEntry, error) &&
                m_archiver->extract(motionEntry, bytes, error) && !bytes.empty() && motion->load(bytes, 0, error);
            if (continuable) {
                motion->setFileURI(URI::createFromFilePath(m_fileURI.absolutePath(), entry.m_path));
                if (name == String("Camera")) {
                    lastMotion = m_project->cameraMotion();
                    m_project->setCameraMotion(motion);
                }
                else if (name == String("Light")) {
                    lastMotion = m_project->lightMotion();
                    m_project->setLightMotion(motion);
                }
                else if (name == String("Shadow")) {
                    lastMotion = m_project->selfShadowMotion();
                    m_project->setSelfShadowMotion(motion);
                }
                else if (Model *model = m_project->findModelByName(name)) {
                    lastMotion = m_project->resolveMotion(model);
                    m_project->addModelMotion(motion, model);
                }
                else if (Accessory *accessory = m_project->findAccessoryByName(name)) {
                    lastMotion = m_project->resolveMotion(accessory);
                    m_project->addAccessoryMotion(motion, accessory);
                }
            }
        }
    }
    m_project->destroyMotion(lastMotion ? lastMotion : motion);
    m_progress->increment();
    return continuable;
}

bool
Archive::loadAllEffects(Native &native, Error &error)
{
    const bool isPluginEnabled = m_project->isEffectPluginEnabled();
    plugin::EffectPlugin *plugin = m_project->fileManager()->sharedEffectPlugin();
    const Project::DrawableList *drawableOrderList = m_project->drawableOrderList();
    bool continuable = true;
    if (isPluginEnabled && plugin) {
        for (Project::DrawableList::const_iterator it = drawableOrderList->begin(), end = drawableOrderList->end();
             it != end && continuable; ++it) {
            IDrawable *drawable = *it;
            continuable &= loadEffectFromSource(drawable->fileURI(), drawable, plugin, native, error);
        }
    }
    if (continuable) {
        for (Project::DrawableList::const_iterator it = drawableOrderList->begin(), end = drawableOrderList->end();
             it != end && continuable; ++it) {
            IDrawable *drawable = *it;
            continuable &= loadEffectFromBinary(drawable->fileURI(), drawable, error);
        }
    }
    if (continuable) {
        continuable &= loadAllOffscreenEffectAttachments(native, plugin, error);
    }
    return continuable;
}

bool
Archive::loadAllOffscreenEffectAttachments(Native &native, plugin::EffectPlugin *plugin, Error &error)
{
    Native::OffscreenRenderTargetEffectAttachmentList attachments;
    native.getAllOffscreenRenderTargetEffectAttachments(attachments);
    bool continuable = true;
    for (Native::OffscreenRenderTargetEffectAttachmentList::const_iterator it = attachments.begin(),
                                                                           end = attachments.end();
         it != end && continuable; ++it) {
        continuable &= loadOffscreenEffectAttachment(*it, plugin, error);
    }
    return continuable;
}

bool
Archive::loadOffscreenEffectAttachment(
    const Native::OffscreenRenderTargetEffectAttachment &attachment, plugin::EffectPlugin *plugin, Error &error)
{
    SG_PUSH_GROUPF("internal::project::Archive::loadOffscreenEffectAttachment(name=%s, filePath=%s, target=%s)",
        attachment.m_name.c_str(), attachment.m_filePath.c_str(), attachment.m_target->nameConstString());
    String entryPath;
    PluginFactory::EffectPluginProxy proxy(plugin);
    Archiver::Entry entry;
    ByteArray inputSource;
    bool continuable = true;
    DrawablePathMap::const_iterator it = m_pathDrawables.find(attachment.m_target);
    if (it != m_pathDrawables.end()) {
        const String basePath(URI::stringByDeletingLastPathComponent(it->second));
        entryPath.append(basePath.c_str());
        entryPath.append("/");
        entryPath.append(attachment.m_filePath.c_str());
        for (StringList::const_iterator it = attachment.m_includePaths.begin(), end = attachment.m_includePaths.end();
             it != end; ++it) {
            String includePath(basePath.c_str());
            includePath.append("/");
            includePath.append(PrivateArchiveUtils::trimMovingDirectoryPath(it->c_str()));
            if (m_archiver->findEntry(includePath, entry, error) && m_archiver->extract(entry, inputSource, error)) {
                proxy.addIncludeSource(*it, inputSource);
            }
        }
    }
    if (m_archiver->findEntry(entryPath, entry, error) && m_archiver->extract(entry, inputSource, error)) {
        const String originSource(reinterpret_cast<const char *>(inputSource.data()), inputSource.size());
        continuable = false;
        ByteArray outputBinary;
        if (!m_progress->tryLoadingItem(URI::createFromFilePath(entryPath))) {
            error = Error::cancelled();
        }
        else if (proxy.compile(originSource, outputBinary)) {
            Effect *effect = m_project->createEffect();
            effect->setName(entry.lastPathComponent());
            if (effect->load(outputBinary, *m_progress, error)) {
                IDrawable *drawable = attachment.m_target;
                effect->setFileURI(URI::createFromFilePath(drawable->fileURI().absolutePath(), entryPath));
                if (effect->upload(effect::kAttachmentTypeOffscreenPassive, m_archiver, *m_progress, error)) {
                    m_project->setOffscreenPassiveRenderTargetEffect(attachment.m_name, drawable, effect);
                    continuable = !error.hasReason();
                }
            }
            if (!continuable) {
                m_project->destroyEffect(effect);
            }
        }
        else {
            error = proxy.error();
        }
    }
    SG_POP_GROUP();
    return continuable;
}

bool
Archive::loadEffectFromSource(
    const URI &fileURI, IDrawable *drawable, plugin::EffectPlugin *plugin, Native &native, Error &error)
{
    SG_PUSH_GROUPF("internal::project::Archive::loadEffectFromSource(fileURI=%s#%s)", fileURI.absolutePathConstString(),
        fileURI.fragmentConstString());
    PluginFactory::EffectPluginProxy proxy(plugin);
    const StringList &extensionList = proxy.availableExtensions();
    const String &fragment = fileURI.fragment();
    String basePath(URI::stringByDeletingLastPathComponent(fragment));
    basePath.append("/");
    Project::IncludeEffectSourceMap includeEffectSources;
    aggregateAllIncludeEffectSources(extensionList, includeEffectSources, error);
    bool continuable = true;
    for (StringList::const_iterator it = extensionList.begin(), end = extensionList.end(); it != end; ++it) {
        const String &effectPath = Effect::resolveFilePath(fragment.c_str(), it->c_str());
        Archiver::Entry entry;
        ByteArray inputSource;
        if (m_archiver->findEntry(effectPath, entry, error) && m_archiver->extract(entry, inputSource, error)) {
            continuable = false;
            addIncludeEffectSource(drawable, basePath, native, proxy, error);
            const String originSource(reinterpret_cast<const char *>(inputSource.data()), inputSource.size());
            ByteArray outputBinary;
            if (!m_progress->tryLoadingItem(fileURI)) {
                error = Error::cancelled();
                break;
            }
            else if (proxy.compile(originSource, outputBinary)) {
                Effect *effect = m_project->createEffect();
                effect->setName(entry.lastPathComponent());
                if (effect->load(outputBinary, *m_progress, error)) {
                    effect->setFileURI(URI::createFromFilePath(fileURI.absolutePath(), effectPath));
                    if (effect->upload(effect::kAttachmentTypeNone, m_archiver, *m_progress, error)) {
                        m_project->attachActiveEffect(drawable, effect, includeEffectSources, *m_progress, error);
                        continuable = !error.hasReason();
                    }
                }
                if (!continuable) {
                    m_project->destroyEffect(effect);
                    break;
                }
            }
            else {
                error = proxy.error();
                break;
            }
        }
    }
    SG_POP_GROUP();
    return continuable;
}

bool
Archive::loadEffectFromBinary(const URI &fileURI, IDrawable *drawable, Error &error)
{
    SG_PUSH_GROUPF("internal::project::Archive::loadEffectFromBinary(fileURI=%s#%s)", fileURI.absolutePathConstString(),
        fileURI.fragmentConstString());
    Archiver::Entry effectEntry;
    ByteArray input;
    const String &effectEntryPath =
        Effect::resolveFilePath(fileURI.fragmentConstString(), Effect::kBinaryFileExtension);
    bool continuable = true;
    if (m_archiver->findEntry(effectEntryPath, effectEntry, error) && m_archiver->extract(effectEntry, input, error)) {
        continuable = false;
        Effect *effect = m_project->createEffect();
        effect->setName(effectEntry.lastPathComponent());
        if (!m_progress->tryLoadingItem(fileURI)) {
            error = Error::cancelled();
        }
        else if (effect->load(input, *m_progress, error)) {
            effect->setFileURI(URI::createFromFilePath(fileURI.absolutePath(), effectEntryPath));
            if (effect->upload(effect::kAttachmentTypeNone, m_archiver, *m_progress, error)) {
                m_project->attachActiveEffect(drawable, effect, Project::IncludeEffectSourceMap(), *m_progress, error);
                continuable = !error.hasReason();
            }
        }
        if (!continuable) {
            m_project->destroyEffect(effect);
        }
    }
    SG_POP_GROUP();
    return continuable;
}

void
Archive::aggregateAllIncludeEffectSources(
    const StringList &extensionList, Project::IncludeEffectSourceMap &includeEffectSources, Error &error)
{
    const StringSet &extensionSet = ListUtils::toSetFromList(extensionList);
    const Archiver::EntryList &allEntries = m_archiver->allEntries(error);
    for (Archiver::EntryList::const_iterator it = allEntries.begin(), end = allEntries.end(); it != end; ++it) {
        const Archiver::Entry &entry = *it;
        const char *extension = entry.extensionPtr();
        if (extension && extensionSet.find(extension) != extensionSet.end()) {
            // Accessory or Model
            const String &path = entry.m_path;
            if (const char *p1 = StringUtils::indexOf(path, '/')) {
                // name of the accessory or the model
                if (const char *p2 = StringUtils::indexOf(p1 + 1, '/')) {
                    ByteArray bytes;
                    Archiver::Entry foundEntry;
                    if (m_archiver->findEntry(path, foundEntry, error) &&
                        m_archiver->extract(foundEntry, bytes, error)) {
                        const String filename(p2 + 1);
                        includeEffectSources.insert(tinystl::make_pair(String(entry.filenamePtr()), bytes));
                        includeEffectSources.insert(tinystl::make_pair(filename, bytes));
                    }
                }
            }
        }
    }
}

void
Archive::addIncludeEffectSource(const IDrawable *drawable, const String &basePath, Native &native,
    PluginFactory::EffectPluginProxy &proxy, Error &error)
{
    if (Project::IncludeEffectSourceMap *includeSourceMap = native.findMutableIncludeEffectSource(drawable)) {
        for (Project::IncludeEffectSourceMap::iterator it2 = includeSourceMap->begin(), end2 = includeSourceMap->end();
             it2 != end2; ++it2) {
            const char *includePath = it2->first.c_str(),
                       *trimmedIncludePath = PrivateArchiveUtils::trimMovingDirectoryPath(includePath);
            String normalizedIncludePath(basePath);
            normalizedIncludePath.append(trimmedIncludePath);
            Archiver::Entry includeEntry;
            ByteArray includeSource;
            if (m_archiver->findEntry(normalizedIncludePath, includeEntry, error) &&
                m_archiver->extract(includeEntry, includeSource, error)) {
                proxy.addIncludeSource(includePath, includeSource);
                it2->second = includeSource;
            }
        }
    }
}

bool
Archive::saveAllAccessories(Native &native, StringSet &reservedNameSet, Error &error)
{
    const Project::AccessoryList *allAccessories = m_project->allAccessories();
    bool continuable = true;
    for (Project::AccessoryList::const_iterator it = allAccessories->begin(), end = allAccessories->end();
         continuable && it != end && continuable; ++it) {
        Accessory *accessory = *it;
        const String &name = accessory->name();
        if (!name.empty()) {
            const String &actualName = Project::resolveNameConfliction(name, reservedNameSet);
            String prefix("Accessory/");
            prefix.append(actualName.c_str());
            prefix.append("/");
            accessory->setName(actualName);
            accessory->setAnnotations(PrivateArchiveUtils::addUUID(m_project, accessory, accessory->annotations()));
            continuable &= accessory->saveArchive(prefix, *m_archiver, error) &&
                saveAllDrawableIncludeSources(accessory, native, error);
            if (continuable) {
                m_accessoryMotionSet.insert(
                    tinystl::make_pair(m_project->resolveMotion(accessory), static_cast<IDrawable *>(accessory)));
            }
        }
    }
    return continuable;
}

bool
Archive::saveAllModels(Native &native, StringSet &reservedNameSet, Error &error)
{
    nanoem_unicode_string_factory_t *factory = m_project->unicodeStringFactory();
    StringUtils::UnicodeStringScope scope(factory);
    const Project::ModelList *allModels = m_project->allModels();
    bool continuable = true;
    for (Project::ModelList::const_iterator it = allModels->begin(), end = allModels->end();
         continuable && it != end && continuable; ++it) {
        Model *model = *it;
        const String &canonicalName = model->canonicalName();
        if (!canonicalName.empty()) {
            const String &actualName = Project::resolveNameConfliction(canonicalName, reservedNameSet);
            String prefix("Model/");
            prefix.append(actualName.c_str());
            prefix.append("/");
            if (!(actualName == canonicalName)) {
                model->setName(actualName);
                model->rename(actualName, NANOEM_LANGUAGE_TYPE_FIRST_ENUM);
            }
            model->setDirty(false);
            model->setAnnotations(PrivateArchiveUtils::addUUID(m_project, model, model->annotations()));
            continuable =
                model->saveArchive(prefix, *m_archiver, error) && saveAllDrawableIncludeSources(model, native, error);
            if (continuable) {
                m_modelMotionSet.insert(
                    tinystl::make_pair(m_project->resolveMotion(model), static_cast<IDrawable *>(model)));
            }
        }
    }
    return continuable;
}

bool
Archive::saveAllMotions(Error &error)
{
    String modelName;
    const Project::MotionList *allMotions = m_project->allMotions();
    ByteArray bytes;
    bool continuable = true;
    for (Project::MotionList::const_iterator it = allMotions->begin(), end = allMotions->end();
         it != end && continuable; ++it) {
        Motion *motion = *it;
        String path("Motion/");
        bool saved = false;
        Archiver::Entry entry;
        motion->setAnnotations(PrivateArchiveUtils::addUUID(m_project, motion, motion->annotations()));
        motion->setFormat(NANOEM_MOTION_FORMAT_TYPE_NMD);
        if (motion == m_project->cameraMotion()) {
            saved = motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_CAMERA, error);
            path.append("Camera");
        }
        else if (motion == m_project->lightMotion()) {
            saved = motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_LIGHT, error);
            path.append("Light");
        }
        else if (motion == m_project->selfShadowMotion()) {
            saved = motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_SELFSHADOW, error);
            path.append("Shadow");
        }
        else if (m_modelMotionSet.find(motion) != m_modelMotionSet.end()) {
            const Model *model = static_cast<const Model *>(m_modelMotionSet.find(motion)->second);
            saved = motion->save(bytes, model, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_MODEL, error);
            path.append(model->canonicalNameConstString());
        }
        else if (m_accessoryMotionSet.find(motion) != m_accessoryMotionSet.end()) {
            const Accessory *accessory = static_cast<const Accessory *>(m_accessoryMotionSet.find(motion)->second);
            saved = motion->save(bytes, nullptr, NANOEM_MUTABLE_MOTION_KEYFRAME_TYPE_ACCESSORY, error);
            path.append(accessory->canonicalNameConstString());
        }
        if (!saved) {
            continue;
        }
        path.append(".");
        path.append(Motion::kNMDFormatExtension.c_str());
        entry.m_path = path;
        continuable &= m_archiver->addEntry(entry, bytes, error);
    }
    return continuable;
}

bool
Archive::saveAudio(Error &error)
{
    IAudioPlayer *audioPlayer = m_project->audioPlayer();
    if (audioPlayer->isLoaded()) {
        IAudioPlayer::Description desc;
        const ByteArray *samplesPtr = audioPlayer->linearPCMSamples();
        BaseAudioPlayer::initializeDescription(audioPlayer, samplesPtr->size(), desc);
        Archiver::Entry entry;
        ByteArray bytes;
        MemoryWriter writer(&bytes);
        FileUtils::writeTyped(&writer, desc, error);
        FileUtils::writeTyped(&writer, nanoem_fourcc('d', 'a', 't', 'a'), error);
        FileUtils::writeTyped(&writer, static_cast<nanoem_u32_t>(samplesPtr->size()), error);
        FileUtils::write(&writer, samplesPtr->data(), samplesPtr->size(), error);
        entry.m_path = kBGMEntryPath;
        m_archiver->addEntry(entry, bytes, error);
    }
    return !error.hasReason();
}

bool
Archive::saveVideo(Error &error)
{
    IBackgroundVideoRenderer *videoRenderer = m_project->backgroundVideoRenderer();
    const URI &fileURI = videoRenderer->fileURI();
    if (!fileURI.isEmpty()) {
        Archiver::Entry entry;
        entry.m_path = "BackGround/";
        entry.m_path.append(fileURI.lastPathComponentConstString());
        FileReaderScope scope(m_project->translator());
        if (scope.open(fileURI, error)) {
            m_archiver->addEntry(entry, scope.reader(), error);
        }
    }
    return !error.hasReason();
}

bool
Archive::saveAllDrawableIncludeSources(IDrawable *drawable, Native &native, Error &error)
{
    if (const Project::IncludeEffectSourceMap *includeSourceMap = native.findIncludeEffectSource(drawable)) {
        const URI &fileURI = drawable->fileURI();
        const String &fragment = fileURI.fragment();
        for (Project::IncludeEffectSourceMap::const_iterator it = includeSourceMap->begin(),
                                                             end = includeSourceMap->end();
             it != end; ++it) {
            const char *includePath = it->first.c_str();
            String archiveIncludePath(URI::stringByDeletingLastPathComponent(fragment));
            archiveIncludePath.append("/");
            archiveIncludePath.append(includePath);
            Archiver::Entry entry;
            ByteArray bytes;
            if (!m_archiver->findEntry(archiveIncludePath, entry, error)) {
                FileReaderScope scope(m_project->translator());
                String fileIncludePath(fileURI.absolutePathByDeletingLastPathComponent());
                fileIncludePath.append("/");
                fileIncludePath.append(includePath);
                if (scope.open(URI::createFromFilePath(fileIncludePath), error)) {
                    FileUtils::read(scope, bytes, error);
                }
            }
            if (!bytes.empty()) {
                m_archiver->addEntry(entry, bytes, error);
            }
        }
    }
    return !error.hasReason();
}

} /* namespace project */
} /* namespace internal */
} /* namespace nanoem */
